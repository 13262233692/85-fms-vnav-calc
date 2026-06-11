#include "vnav_solver.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace fmc {
namespace nav {

VNAVSolver::VNAVSolver()
    : m_cruiseAltitudeFt(35000.0) {
}

void VNAVSolver::setAircraftPerformance(const AircraftPerformance& perf) {
    m_performance = perf;
}

void VNAVSolver::setFlightPlan(const FlightPlan& plan) {
    m_flightPlan = plan;
}

void VNAVSolver::setAltitudeConstraints(const std::vector<AltitudeConstraint>& constraints) {
    m_constraints = constraints;
}

void VNAVSolver::setCruiseAltitude(double cruiseAltFt) {
    m_cruiseAltitudeFt = cruiseAltFt;
}

void VNAVSolver::clamp(double& value, double lo, double hi) const {
    if (value < lo) value = lo;
    if (value > hi) value = hi;
}

double VNAVSolver::gradientLimitClimb(double distanceNm) const {
    return m_performance.maxClimbGradientFtPerNm * std::max(0.0, distanceNm);
}

double VNAVSolver::gradientLimitDescent(double distanceNm) const {
    return m_performance.maxDescentGradientFtPerNm * std::max(0.0, distanceNm);
}

double VNAVSolver::safeClimbFrom(double prevAlt, double distanceNm) const {
    return prevAlt + gradientLimitClimb(distanceNm);
}

double VNAVSolver::safeDescentFrom(double prevAlt, double distanceNm) const {
    return std::max(0.0, prevAlt - gradientLimitDescent(distanceNm));
}

double VNAVSolver::safeClimbTo(double nextAlt, double distanceNm) const {
    return nextAlt - gradientLimitClimb(distanceNm);
}

double VNAVSolver::safeDescentTo(double nextAlt, double distanceNm) const {
    return nextAlt + gradientLimitDescent(distanceNm);
}

void VNAVSolver::initializeWaypoints() {
    const auto& legs = m_flightPlan.legs;
    m_result.waypoints.clear();
    m_result.waypoints.reserve(legs.size());

    for (size_t i = 0; i < legs.size(); ++i) {
        VNAVWaypointSolution wp;
        wp.legIndex = i;
        wp.waypointIdentifier = legs[i].waypointIdentifier;
        wp.cumulativeDistanceNm = legs[i].cumulativeDistance;
        wp.distanceFromPreviousNm = legs[i].distanceFromOrigin;
        if (i > 0) {
            wp.distanceFromPreviousNm = legs[i].cumulativeDistance - legs[i-1].cumulativeDistance;
        }

        wp.minAllowedAltitudeFt = -std::numeric_limits<double>::infinity();
        wp.maxAllowedAltitudeFt = std::numeric_limits<double>::infinity();
        wp.plannedAltitudeFt = 0.0;
        wp.climbLimitFt = 0.0;
        wp.descentLimitFt = std::numeric_limits<double>::infinity();
        wp.phase = VerticalFlightPhase::Unknown;
        wp.verticalSpeedFpm = 0.0;
        wp.flightPathAngleDeg = 0.0;
        wp.timeToWaypointMin = 0.0;
        wp.constraintViolated = false;
        wp.violationMessage = "";

        if (i < m_constraints.size()) {
            wp.originalConstraint = m_constraints[i];
            wp.minAllowedAltitudeFt = std::max(wp.minAllowedAltitudeFt, m_constraints[i].getMinAllowed());
            wp.maxAllowedAltitudeFt = std::min(wp.maxAllowedAltitudeFt, m_constraints[i].getMaxAllowed());
        }

        if (i == 0) {
            if (wp.minAllowedAltitudeFt == -std::numeric_limits<double>::infinity()) {
                wp.minAllowedAltitudeFt = 100.0;
            }
            if (wp.maxAllowedAltitudeFt == std::numeric_limits<double>::infinity()) {
                wp.maxAllowedAltitudeFt = std::max(wp.minAllowedAltitudeFt, 500.0);
            }
            wp.plannedAltitudeFt = (wp.minAllowedAltitudeFt + wp.maxAllowedAltitudeFt) / 2.0;
            clamp(wp.plannedAltitudeFt, wp.minAllowedAltitudeFt, wp.maxAllowedAltitudeFt);
        } else if (i == legs.size() - 1) {
            if (wp.minAllowedAltitudeFt == -std::numeric_limits<double>::infinity()) {
                wp.minAllowedAltitudeFt = 100.0;
            }
            if (wp.maxAllowedAltitudeFt == std::numeric_limits<double>::infinity()) {
                wp.maxAllowedAltitudeFt = std::max(wp.minAllowedAltitudeFt, 500.0);
            }
            wp.plannedAltitudeFt = (wp.minAllowedAltitudeFt + wp.maxAllowedAltitudeFt) / 2.0;
            clamp(wp.plannedAltitudeFt, wp.minAllowedAltitudeFt, wp.maxAllowedAltitudeFt);
        }

        m_result.waypoints.push_back(wp);
    }

    m_result.totalDistanceNm = m_flightPlan.totalDistanceNm;
}

void VNAVSolver::applyClimbLimitsForward() {
    auto& wps = m_result.waypoints;
    if (wps.empty()) return;

    wps[0].climbLimitFt = wps[0].plannedAltitudeFt;

    for (size_t i = 1; i < wps.size(); ++i) {
        double dist = wps[i].distanceFromPreviousNm;
        if (dist <= 0.0) dist = 0.001;

        double maxReachableByClimb = safeClimbFrom(wps[i-1].climbLimitFt, dist);
        wps[i].climbLimitFt = maxReachableByClimb;

        if (wps[i].minAllowedAltitudeFt != -std::numeric_limits<double>::infinity()) {
            if (wps[i].climbLimitFt < wps[i].minAllowedAltitudeFt - TOLERANCE_FT) {
                m_result.gradientTruncationApplied = true;
                std::ostringstream oss;
                oss << "WP " << wps[i].waypointIdentifier
                    << ": 爬升梯度限制 - 需要 " << std::fixed << std::setprecision(0)
                    << wps[i].minAllowedAltitudeFt << "ft 但仅能到达 "
                    << std::fixed << std::setprecision(0) << wps[i].climbLimitFt << "ft"
                    << " (距离 " << std::fixed << std::setprecision(1) << dist << "NM)";
                m_result.warnings.push_back(oss.str());
            }
        }
    }
}

void VNAVSolver::applyDescentLimitsBackward() {
    auto& wps = m_result.waypoints;
    if (wps.empty()) return;

    size_t n = wps.size();
    wps[n-1].descentLimitFt = wps[n-1].plannedAltitudeFt;

    for (size_t i = n - 1; i > 0; --i) {
        size_t j = i - 1;
        double dist = wps[i].distanceFromPreviousNm;
        if (dist <= 0.0) dist = 0.001;

        double maxAltBeforeDescent = safeDescentTo(wps[i].descentLimitFt, dist);
        wps[j].descentLimitFt = std::min(wps[j].descentLimitFt, maxAltBeforeDescent);

        if (wps[j].maxAllowedAltitudeFt != std::numeric_limits<double>::infinity()) {
            if (wps[j].descentLimitFt < wps[j].maxAllowedAltitudeFt - TOLERANCE_FT) {
            }
        }
    }

    for (size_t i = 0; i < n; ++i) {
        if (wps[i].maxAllowedAltitudeFt != std::numeric_limits<double>::infinity()) {
            if (wps[i].descentLimitFt > wps[i].maxAllowedAltitudeFt + TOLERANCE_FT) {
                m_result.gradientTruncationApplied = true;
                std::ostringstream oss;
                oss << "WP " << wps[i].waypointIdentifier
                    << ": 下降梯度限制 - 最大允许 " << std::fixed << std::setprecision(0)
                    << wps[i].maxAllowedAltitudeFt << "ft 但需要至少 "
                    << std::fixed << std::setprecision(0) << wps[i].descentLimitFt << "ft"
                    << " 才能满足后续约束";
                m_result.warnings.push_back(oss.str());
            }
        }
    }
}

void VNAVSolver::enforceConstraintBounds() {
    auto& wps = m_result.waypoints;
    size_t n = wps.size();

    for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
        bool changed = false;

        for (size_t i = 0; i < n; ++i) {
            double oldMin = wps[i].minAllowedAltitudeFt;
            double oldMax = wps[i].maxAllowedAltitudeFt;

            if (wps[i].minAllowedAltitudeFt != -std::numeric_limits<double>::infinity()) {
                wps[i].climbLimitFt = std::max(wps[i].climbLimitFt, wps[i].minAllowedAltitudeFt);
            }

            if (wps[i].maxAllowedAltitudeFt != std::numeric_limits<double>::infinity()) {
                wps[i].descentLimitFt = std::min(wps[i].descentLimitFt, wps[i].maxAllowedAltitudeFt);
            }

            double physMin = wps[i].climbLimitFt;
            double physMax = wps[i].descentLimitFt;

            if (physMin > physMax + TOLERANCE_FT * 10) {
                wps[i].constraintViolated = true;
                std::ostringstream oss;
                oss << "WP " << wps[i].waypointIdentifier
                    << ": 物理约束冲突 - 爬升下限 " << std::fixed << std::setprecision(0) << physMin
                    << "ft > 下降上限 " << std::fixed << std::setprecision(0) << physMax << "ft";
                wps[i].violationMessage = oss.str();
            }

            if (wps[i].minAllowedAltitudeFt != -std::numeric_limits<double>::infinity()) {
                double newMin = std::max(wps[i].minAllowedAltitudeFt, physMin);
                if (std::abs(newMin - oldMin) > TOLERANCE_FT) {
                    changed = true;
                }
            }
            if (wps[i].maxAllowedAltitudeFt != std::numeric_limits<double>::infinity()) {
                double newMax = std::min(wps[i].maxAllowedAltitudeFt, physMax);
                if (std::abs(newMax - oldMax) > TOLERANCE_FT) {
                    changed = true;
                }
            }
        }

        if (!changed) break;
    }
}

void VNAVSolver::computePlannedAltitudes() {
    auto& wps = m_result.waypoints;
    size_t n = wps.size();
    if (n == 0) return;

    double cruiseAlt = m_cruiseAltitudeFt;

    for (size_t i = 0; i < n; ++i) {
        double physMin = wps[i].climbLimitFt;
        double physMax = wps[i].descentLimitFt;

        double userMin = (wps[i].minAllowedAltitudeFt == -std::numeric_limits<double>::infinity())
                         ? physMin : wps[i].minAllowedAltitudeFt;
        double userMax = (wps[i].maxAllowedAltitudeFt == std::numeric_limits<double>::infinity())
                         ? physMax : wps[i].maxAllowedAltitudeFt;

        double lo = std::max(physMin, userMin);
        double hi = std::min(physMax, userMax);

        if (lo > hi) {
            lo = std::min(physMin, userMin);
            hi = std::max(physMax, userMax);
            if (hi < lo) std::swap(lo, hi);
        }

        if (wps[i].originalConstraint.type == AltitudeConstraintType::At) {
            double atAlt = wps[i].originalConstraint.altitudeAtFt;
            if (atAlt >= lo - TOLERANCE_FT * 5 && atAlt <= hi + TOLERANCE_FT * 5) {
                wps[i].plannedAltitudeFt = atAlt;
                continue;
            } else {
                wps[i].constraintViolated = true;
                if (wps[i].violationMessage.empty()) {
                    std::ostringstream oss;
                    oss << "WP " << wps[i].waypointIdentifier
                        << ": AT约束 " << std::fixed << std::setprecision(0) << atAlt
                        << "ft 不可实现，调整至可行区间 ["
                        << std::fixed << std::setprecision(0) << lo << ", "
                        << std::fixed << std::setprecision(0) << hi << "]";
                    wps[i].violationMessage = oss.str();
                }
            }
        }

        if (cruiseAlt >= lo - TOLERANCE_FT && cruiseAlt <= hi + TOLERANCE_FT) {
            wps[i].plannedAltitudeFt = cruiseAlt;
        } else if (cruiseAlt > hi) {
            wps[i].plannedAltitudeFt = hi;
        } else {
            wps[i].plannedAltitudeFt = lo;
        }

        clamp(wps[i].plannedAltitudeFt, lo, hi);
    }

    for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
        bool feasible = true;

        for (size_t i = 1; i < n; ++i) {
            double dist = wps[i].distanceFromPreviousNm;
            if (dist <= 0.0) dist = 0.001;

            double maxAltNext = safeClimbFrom(wps[i-1].plannedAltitudeFt, dist);
            if (wps[i].plannedAltitudeFt > maxAltNext + TOLERANCE_FT) {
                wps[i].plannedAltitudeFt = maxAltNext;
                feasible = false;
            }

            double minAltPrev = safeClimbTo(wps[i].plannedAltitudeFt, dist);
            if (wps[i-1].plannedAltitudeFt < minAltPrev - TOLERANCE_FT) {
                wps[i-1].plannedAltitudeFt = minAltPrev;
                feasible = false;
            }
        }

        for (size_t i = n - 1; i > 0; --i) {
            size_t j = i - 1;
            double dist = wps[i].distanceFromPreviousNm;
            if (dist <= 0.0) dist = 0.001;

            double minAltPrev = safeDescentTo(wps[i].plannedAltitudeFt, dist);
            if (wps[j].plannedAltitudeFt > minAltPrev + TOLERANCE_FT) {
                wps[j].plannedAltitudeFt = minAltPrev;
                feasible = false;
            }

            double maxAltNext = safeDescentFrom(wps[j].plannedAltitudeFt, dist);
            if (wps[i].plannedAltitudeFt < maxAltNext - TOLERANCE_FT) {
                wps[i].plannedAltitudeFt = maxAltNext;
                feasible = false;
            }
        }

        if (feasible) break;
    }
}

void VNAVSolver::detectPhaseTransitions() {
    auto& wps = m_result.waypoints;
    size_t n = wps.size();
    if (n == 0) return;

    m_result.topOfClimbDistanceNm = 0.0;
    m_result.topOfDescentDistanceNm = m_result.totalDistanceNm;

    for (size_t i = 0; i < n; ++i) {
        if (i == 0) {
            wps[i].phase = VerticalFlightPhase::Climb;
            continue;
        }

        double altChange = wps[i].plannedAltitudeFt - wps[i-1].plannedAltitudeFt;
        double dist = wps[i].distanceFromPreviousNm;
        if (dist <= 0.0) dist = 0.001;

        double gradFtPerNm = altChange / dist;

        if (std::abs(gradFtPerNm) < 20.0) {
            wps[i].phase = VerticalFlightPhase::Cruise;
            if (m_result.topOfClimbDistanceNm == 0.0 && i > 1) {
                m_result.topOfClimbDistanceNm = wps[i-1].cumulativeDistanceNm;
            }
        } else if (gradFtPerNm > 0) {
            wps[i].phase = VerticalFlightPhase::Climb;
        } else {
            wps[i].phase = VerticalFlightPhase::Descent;
            if (m_result.topOfDescentDistanceNm == m_result.totalDistanceNm) {
                m_result.topOfDescentDistanceNm = wps[i-1].cumulativeDistanceNm;
            }
        }
    }

    double maxAlt = 0.0;
    for (const auto& wp : wps) {
        maxAlt = std::max(maxAlt, wp.plannedAltitudeFt);
    }
    m_result.cruiseAltitudeFt = maxAlt;
}

void VNAVSolver::checkFeasibilityAndRelax() {
    auto& wps = m_result.waypoints;
    size_t n = wps.size();

    for (size_t i = 0; i < n; ++i) {
        const auto& c = wps[i].originalConstraint;
        double planned = wps[i].plannedAltitudeFt;
        bool violated = false;
        std::string msg;

        switch (c.type) {
            case AltitudeConstraintType::At:
                if (std::abs(planned - c.altitudeAtFt) > TOLERANCE_FT * 5) {
                    violated = true;
                    std::ostringstream oss;
                    oss << "WP " << wps[i].waypointIdentifier
                        << ": AT约束 " << std::fixed << std::setprecision(0) << c.altitudeAtFt
                        << "ft 已放松，实际规划 " << std::fixed << std::setprecision(0) << planned << "ft";
                    msg = oss.str();
                }
                break;
            case AltitudeConstraintType::AtOrAbove:
                if (planned < c.altitudeAboveFt - TOLERANCE_FT * 5) {
                    violated = true;
                    std::ostringstream oss;
                    oss << "WP " << wps[i].waypointIdentifier
                        << ": ABOVE约束 " << std::fixed << std::setprecision(0) << c.altitudeAboveFt
                        << "ft 已放松，实际规划 " << std::fixed << std::setprecision(0) << planned << "ft";
                    msg = oss.str();
                }
                break;
            case AltitudeConstraintType::AtOrBelow:
                if (planned > c.altitudeBelowFt + TOLERANCE_FT * 5) {
                    violated = true;
                    std::ostringstream oss;
                    oss << "WP " << wps[i].waypointIdentifier
                        << ": BELOW约束 " << std::fixed << std::setprecision(0) << c.altitudeBelowFt
                        << "ft 已放松，实际规划 " << std::fixed << std::setprecision(0) << planned << "ft";
                    msg = oss.str();
                }
                break;
            case AltitudeConstraintType::Between:
                if (planned < c.altitudeAboveFt - TOLERANCE_FT * 5 ||
                    planned > c.altitudeBelowFt + TOLERANCE_FT * 5) {
                    violated = true;
                    std::ostringstream oss;
                    oss << "WP " << wps[i].waypointIdentifier
                        << ": BETWEEN约束 [" << std::fixed << std::setprecision(0) << c.altitudeAboveFt
                        << ", " << std::fixed << std::setprecision(0) << c.altitudeBelowFt
                        << "]ft 已放松，实际规划 " << std::fixed << std::setprecision(0) << planned << "ft";
                    msg = oss.str();
                }
                break;
            default:
                break;
        }

        if (violated) {
            wps[i].constraintViolated = true;
            if (wps[i].violationMessage.empty()) {
                wps[i].violationMessage = msg;
            }
            m_result.warnings.push_back(msg);
            if (c.isHardConstraint) {
                m_result.numConstraintsRelaxed++;
            }
        }
    }
}

void VNAVSolver::computeVerticalParameters() {
    auto& wps = m_result.waypoints;
    size_t n = wps.size();

    for (size_t i = 0; i < n; ++i) {
        double speedKtas = m_performance.cruiseSpeedKtas;
        if (wps[i].phase == VerticalFlightPhase::Descent) {
            speedKtas = m_performance.descentSpeedKtas;
        }
        if (i == n - 1) {
            speedKtas = m_performance.approachSpeedKtas;
        }

        if (i > 0) {
            double dist = wps[i].distanceFromPreviousNm;
            if (dist <= 0.0) dist = 0.001;

            double altChangeFt = wps[i].plannedAltitudeFt - wps[i-1].plannedAltitudeFt;
            double timeMin = dist / speedKtas * 60.0;
            if (timeMin <= 0.0) timeMin = 0.001;

            wps[i].verticalSpeedFpm = altChangeFt / timeMin;

            double horizFt = dist * 6076.12;
            if (horizFt > 0.0) {
                wps[i].flightPathAngleDeg = std::atan(altChangeFt / horizFt) * RAD_TO_DEG;
            }

            wps[i].timeToWaypointMin = wps[i-1].timeToWaypointMin + timeMin;
        }
    }
}

VNAVProfile VNAVSolver::solve() {
    m_result = VNAVProfile();

    if (m_flightPlan.legs.size() < 2) {
        m_result.status = VNAVSolverStatus::InvalidInput;
        m_result.errors.push_back("飞行计划至少需要2个航点");
        return m_result;
    }

    if (m_constraints.size() > m_flightPlan.legs.size()) {
        m_result.status = VNAVSolverStatus::InvalidInput;
        m_result.errors.push_back("约束数量不能超过航点数量");
        return m_result;
    }

    try {
        initializeWaypoints();
        applyClimbLimitsForward();
        applyDescentLimitsBackward();
        enforceConstraintBounds();
        computePlannedAltitudes();
        detectPhaseTransitions();
        checkFeasibilityAndRelax();
        computeVerticalParameters();

        bool hasErrors = false;
        int violationCount = 0;
        for (const auto& wp : m_result.waypoints) {
            if (wp.constraintViolated) {
                violationCount++;
                if (wp.originalConstraint.isHardConstraint &&
                    wp.originalConstraint.type == AltitudeConstraintType::At) {
                    hasErrors = true;
                    m_result.errors.push_back(wp.violationMessage);
                }
            }
        }

        if (hasErrors && m_result.numConstraintsRelaxed > m_flightPlan.legs.size() / 2) {
            m_result.status = VNAVSolverStatus::ConstraintInfeasible;
        } else if (!m_result.warnings.empty() || m_result.numConstraintsRelaxed > 0) {
            m_result.status = VNAVSolverStatus::SuccessWithWarnings;
        } else {
            m_result.status = VNAVSolverStatus::Success;
        }

    } catch (const std::exception& e) {
        m_result.status = VNAVSolverStatus::InternalError;
        m_result.errors.push_back(std::string("求解器内部错误: ") + e.what());
    } catch (...) {
        m_result.status = VNAVSolverStatus::InternalError;
        m_result.errors.push_back("求解器发生未知错误");
    }

    return m_result;
}

}
}
