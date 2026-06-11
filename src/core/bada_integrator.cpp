#include "bada_integrator.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fmc {
namespace bada {

BADATrajectoryIntegrator::BADATrajectoryIntegrator()
    : m_initialFuelKg(15000.0),
      m_initialMassKg(65000.0),
      m_cruiseMach(0.785),
      m_integrationStepNm(DEFAULT_STEP_NM),
      m_costIndex(30.0) {
}

void BADATrajectoryIntegrator::setAircraft(const BADAAircraft& aircraft) {
    m_aircraft = aircraft;
}

void BADATrajectoryIntegrator::setWindModel(const WindModel& wind) {
    m_wind = wind;
}

void BADATrajectoryIntegrator::setFlightPlan(const nav::FlightPlan& flightPlan) {
    m_flightPlan = flightPlan;
}

void BADATrajectoryIntegrator::setVNAVProfile(const nav::VNAVProfile& vnavProfile) {
    m_vnavProfile = vnavProfile;
}

void BADATrajectoryIntegrator::setInitialFuelKg(double fuelKg) {
    m_initialFuelKg = std::max(0.0, fuelKg);
}

void BADATrajectoryIntegrator::setInitialMassKg(double massKg) {
    m_initialMassKg = std::max(m_aircraft.limits.operatingEmptyWeightKg, massKg);
}

void BADATrajectoryIntegrator::setCruiseMach(double mach) {
    m_cruiseMach = std::max(0.1, std::min(m_aircraft.limits.maxMach, mach));
}

void BADATrajectoryIntegrator::setIntegrationStepNm(double stepNm) {
    m_integrationStepNm = std::max(0.1, std::min(10.0, stepNm));
}

void BADATrajectoryIntegrator::setCostIndex(double costIndex) {
    m_costIndex = std::max(0.0, std::min(200.0, costIndex));
}

double BADATrajectoryIntegrator::interpolate(double x0, double y0, double x1, double y1, double x) const {
    if (std::abs(x1 - x0) < 1e-9) return (y0 + y1) * 0.5;
    double t = (x - x0) / (x1 - x0);
    return y0 + t * (y1 - y0);
}

double BADATrajectoryIntegrator::getAltitudeAtDistanceNm(double distanceNm) const {
    if (m_vnavProfile.waypoints.empty()) {
        if (m_flightPlan.legs.empty()) return 0.0;
        if (m_flightPlan.legs.size() == 1) return m_flightPlan.legs[0].altitude.value_or(35000.0);
        double totalDist = m_flightPlan.totalDistanceNm;
        if (totalDist <= 0.0) return 35000.0;
        double t = distanceNm / totalDist;
        double alt0 = m_flightPlan.legs.front().altitude.value_or(200.0);
        double alt1 = m_flightPlan.legs.back().altitude.value_or(200.0);
        return interpolate(0.0, alt0, totalDist, 36000.0, std::min(distanceNm, totalDist * 0.5))
             + interpolate(totalDist * 0.5, 36000.0, totalDist, alt1, std::max(distanceNm, totalDist * 0.5));
    }

    const auto& wps = m_vnavProfile.waypoints;
    if (distanceNm <= wps.front().cumulativeDistanceNm) {
        return wps.front().plannedAltitudeFt;
    }
    if (distanceNm >= wps.back().cumulativeDistanceNm) {
        return wps.back().plannedAltitudeFt;
    }

    for (size_t i = 1; i < wps.size(); ++i) {
        if (distanceNm <= wps[i].cumulativeDistanceNm) {
            return interpolate(
                wps[i-1].cumulativeDistanceNm, wps[i-1].plannedAltitudeFt,
                wps[i].cumulativeDistanceNm, wps[i].plannedAltitudeFt,
                distanceNm);
        }
    }
    return wps.back().plannedAltitudeFt;
}

FlightPhase BADATrajectoryIntegrator::getPhaseAtDistanceNm(double distanceNm) const {
    if (m_vnavProfile.waypoints.empty()) {
        double total = m_flightPlan.totalDistanceNm;
        if (total <= 0.0) return FlightPhase::Cruise;
        double t = distanceNm / total;
        if (t < 0.2) return FlightPhase::Climb;
        if (t < 0.8) return FlightPhase::Cruise;
        return FlightPhase::Descent;
    }

    const auto& wps = m_vnavProfile.waypoints;
    if (distanceNm <= wps.front().cumulativeDistanceNm) {
        return FlightPhase::Climb;
    }
    if (distanceNm >= wps.back().cumulativeDistanceNm) {
        return FlightPhase::Landing;
    }

    for (size_t i = 1; i < wps.size(); ++i) {
        if (distanceNm <= wps[i].cumulativeDistanceNm) {
            nav::VerticalFlightPhase vphase = wps[i].phase;
            switch (vphase) {
                case nav::VerticalFlightPhase::Climb:    return FlightPhase::Climb;
                case nav::VerticalFlightPhase::Cruise:   return FlightPhase::Cruise;
                case nav::VerticalFlightPhase::Descent:  return FlightPhase::Descent;
                case nav::VerticalFlightPhase::LevelOff: return FlightPhase::Cruise;
                default: return FlightPhase::Cruise;
            }
        }
    }
    return FlightPhase::Cruise;
}

double BADATrajectoryIntegrator::getTrueTrackAtDistanceNm(double distanceNm) const {
    if (m_flightPlan.legs.empty()) return 0.0;
    if (m_flightPlan.legs.size() == 1) return 0.0;

    const auto& legs = m_flightPlan.legs;
    if (distanceNm <= 0.0) return legs[0].trueTrack;
    if (distanceNm >= m_flightPlan.totalDistanceNm) return legs.back().trueTrack;

    for (size_t i = 1; i < legs.size(); ++i) {
        if (distanceNm <= legs[i].cumulativeDistance) {
            double t = (distanceNm - legs[i-1].cumulativeDistance) /
                       std::max(0.001, legs[i].distanceFromOrigin);
            double track = legs[i-1].trueTrack + t * (legs[i].trueTrack - legs[i-1].trueTrack);
            if (track < 0.0) track += 360.0;
            if (track >= 360.0) track -= 360.0;
            return track;
        }
    }
    return legs.back().trueTrack;
}

double BADATrajectoryIntegrator::calculateTargetTasKT(double altitudeFt, FlightPhase phase,
                                                      double& outMach) const {
    auto atm = ISAAtmosphere::atPressureAltitudeFt(altitudeFt);
    double transition = m_aircraft.performance.transitionAltitudeFt;
    double crossover = m_aircraft.performance.crossOverAltitudeFt;

    double tasKT;
    double mach;

    if (phase == FlightPhase::Climb || phase == FlightPhase::Takeoff) {
        if (altitudeFt <= transition) {
            tasKT = 250.0;
            mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
        } else if (altitudeFt <= crossover) {
            tasKT = m_aircraft.performance.standardClimbSpeedScheduleKcas;
            mach = ISAAtmosphere::calculateMachFromTAS(
                ISAAtmosphere::calculateCASToTAS(tasKT, atm), atm);
            tasKT = ISAAtmosphere::calculateCASToTAS(tasKT, atm);
        } else {
            mach = std::min(m_aircraft.performance.standardClimbMach, m_cruiseMach);
            tasKT = ISAAtmosphere::calculateTASFromMach(mach, atm);
        }
    } else if (phase == FlightPhase::Cruise) {
        mach = m_cruiseMach;
        tasKT = ISAAtmosphere::calculateTASFromMach(mach, atm);
    } else if (phase == FlightPhase::Descent) {
        if (altitudeFt >= crossover) {
            mach = m_aircraft.performance.standardDescentMach;
            tasKT = ISAAtmosphere::calculateTASFromMach(mach, atm);
        } else if (altitudeFt >= transition) {
            tasKT = m_aircraft.performance.standardDescentSpeedScheduleKcas;
            tasKT = ISAAtmosphere::calculateCASToTAS(tasKT, atm);
            mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
        } else {
            tasKT = 250.0;
            tasKT = ISAAtmosphere::calculateCASToTAS(tasKT, atm);
            mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
        }
    } else if (phase == FlightPhase::Approach) {
        tasKT = 180.0;
        tasKT = ISAAtmosphere::calculateCASToTAS(tasKT, atm);
        mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
    } else if (phase == FlightPhase::Landing) {
        tasKT = 140.0;
        tasKT = ISAAtmosphere::calculateCASToTAS(tasKT, atm);
        mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
    } else {
        tasKT = 250.0;
        mach = 0.4;
    }

    double maxTasKT = ISAAtmosphere::calculateTASFromMach(m_aircraft.limits.maxMach, atm);
    if (m_aircraft.limits.vmoKcas > 0.0 && altitudeFt <= 10000.0) {
        maxTasKT = std::min(maxTasKT,
            ISAAtmosphere::calculateCASToTAS(m_aircraft.limits.maxSpeedKcasBelow10kFt, atm));
    }

    if (tasKT > maxTasKT) {
        tasKT = maxTasKT;
        mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);
    }

    outMach = mach;
    return tasKT;
}

IntegrationStep BADATrajectoryIntegrator::createInitialStep() {
    IntegrationStep s;
    s.distanceNm = 0.0;
    s.altitudeFt = getAltitudeAtDistanceNm(0.0);
    s.trueTrackDeg = getTrueTrackAtDistanceNm(0.0);
    s.phase = getPhaseAtDistanceNm(0.0);
    s.massKg = m_initialMassKg;
    s.fuelRemainingKg = m_initialFuelKg;
    s.elapsedTimeMin = 0.0;
    s.temperatureOffsetK = m_wind.getTemperatureOffsetAtAltitudeFt(s.altitudeFt);

    s.atmosphere = ISAAtmosphere::atPressureAltitudeFt(s.altitudeFt, s.temperatureOffsetK);

    double mach;
    s.tasKT = calculateTargetTasKT(s.altitudeFt, s.phase, mach);
    s.mach = mach;

    WindVector wind = m_wind.getWindAtAltitudeFt(s.altitudeFt);
    s.gsKT = GroundSpeedCalculator::calculateGroundSpeedKT(
        s.tasKT, s.trueTrackDeg, wind,
        &s.driftAngleDeg, &s.headwindKT, &s.crosswindKT);

    s.dragN = BADAAerodynamics::calculateTotalDrag(
        m_aircraft, s.tasKT, s.altitudeFt, s.massKg, s.phase);

    s.thrustN = std::max(
        BADAAerodynamics::calculateMaximumThrust(m_aircraft, s.altitudeFt, s.phase),
        s.dragN);

    s.liftCoefficient = BADAAerodynamics::calculateLiftCoefficient(
        m_aircraft, s.tasKT, s.altitudeFt, s.massKg);

    s.fuelFlowKgMin = BADAAerodynamics::calculateFuelFlowKgMin(
        m_aircraft, s.thrustN, s.altitudeFt, s.mach, s.phase);

    s.verticalSpeedFpm = 0.0;
    s.flightPathAngleDeg = 0.0;

    return s;
}

void BADATrajectoryIntegrator::integrateStep(IntegrationStep& s, double nextDistanceNm) {
    double stepNm = nextDistanceNm - s.distanceNm;
    if (stepNm <= 0.0) return;

    double nextAlt = getAltitudeAtDistanceNm(nextDistanceNm);
    double altChangeFt = nextAlt - s.altitudeFt;

    FlightPhase nextPhase = getPhaseAtDistanceNm(nextDistanceNm);
    double nextTrack = getTrueTrackAtDistanceNm(nextDistanceNm);
    double tempOffset = m_wind.getTemperatureOffsetAtAltitudeFt(nextAlt);

    auto nextAtm = ISAAtmosphere::atPressureAltitudeFt(nextAlt, tempOffset);

    double mach;
    double nextTas = calculateTargetTasKT(nextAlt, nextPhase, mach);

    WindVector nextWind = m_wind.getWindAtAltitudeFt(nextAlt);

    double avgTas = (s.tasKT + nextTas) * 0.5;
    double avgAlt = (s.altitudeFt + nextAlt) * 0.5;
    double avgTrack = (s.trueTrackDeg + nextTrack) * 0.5;
    auto avgAtm = ISAAtmosphere::atPressureAltitudeFt(avgAlt,
        (s.temperatureOffsetK + tempOffset) * 0.5);
    WindVector avgWind;
    avgWind.directionDegTrue = (s.driftAngleDeg + (s.trueTrackDeg - s.driftAngleDeg) +
                                nextTrack) * 0.333;
    avgWind.speedKT = (s.headwindKT + s.crosswindKT) * 0.5;

    double avgDrift, avgHw, avgCw;
    double avgGs = GroundSpeedCalculator::calculateGroundSpeedKT(
        avgTas, avgTrack, m_wind.getWindAtAltitudeFt(avgAlt),
        &avgDrift, &avgHw, &avgCw);

    double timeMin = stepNm / std::max(0.001, avgGs) * 60.0;
    if (timeMin <= 0.0) timeMin = 0.001;

    double vertSpd = altChangeFt / timeMin;
    double flightPathRad = std::atan2(altChangeFt / 6076.12, stepNm);
    double flightPathDeg = flightPathRad * RAD_TO_DEG;

    double avgMass = s.massKg - s.fuelFlowKgMin * timeMin * 0.5;

    double dragN = BADAAerodynamics::calculateTotalDrag(
        m_aircraft, avgTas, avgAlt, avgMass, nextPhase);

    double thrustN;
    if (nextPhase == FlightPhase::Descent) {
        thrustN = BADAAerodynamics::calculateMaximumThrust(m_aircraft, avgAlt, nextPhase);
    } else if (nextPhase == FlightPhase::Climb) {
        thrustN = BADAAerodynamics::calculateMaximumThrust(m_aircraft, avgAlt, nextPhase);
    } else {
        thrustN = dragN;
    }

    double fuelFlowKgMin = BADAAerodynamics::calculateFuelFlowKgMin(
        m_aircraft, thrustN, avgAlt,
        ISAAtmosphere::calculateMachFromTAS(avgTas, avgAtm), nextPhase);

    double fuelBurned = fuelFlowKgMin * timeMin;
    double newFuel = std::max(0.0, s.fuelRemainingKg - fuelBurned);
    double newMass = s.massKg - fuelBurned;

    if (newFuel <= 0.0 && m_result.warnings.size() < 10) {
        std::ostringstream oss;
        oss << "NM " << std::fixed << std::setprecision(1) << nextDistanceNm
            << ": 燃油耗尽预警！剩余油量不足。";
        m_result.warnings.push_back(oss.str());
    }

    s.distanceNm = nextDistanceNm;
    s.altitudeFt = nextAlt;
    s.tasKT = nextTas;
    s.mach = mach;
    s.trueTrackDeg = nextTrack;
    s.phase = nextPhase;
    s.atmosphere = nextAtm;
    s.temperatureOffsetK = tempOffset;
    s.driftAngleDeg = avgDrift;
    s.headwindKT = avgHw;
    s.crosswindKT = avgCw;
    s.gsKT = avgGs;
    s.verticalSpeedFpm = vertSpd;
    s.flightPathAngleDeg = flightPathDeg;
    s.dragN = dragN;
    s.thrustN = thrustN;
    s.liftCoefficient = BADAAerodynamics::calculateLiftCoefficient(
        m_aircraft, avgTas, avgAlt, avgMass);
    s.fuelFlowKgMin = fuelFlowKgMin;
    s.fuelRemainingKg = newFuel;
    s.massKg = newMass;
    s.elapsedTimeMin += timeMin;
}

void BADATrajectoryIntegrator::integrateLeg(size_t legIndex, LegIntegrationResult& leg) {
    if (m_flightPlan.legs.empty() || legIndex >= m_flightPlan.legs.size()) return;

    const auto& planLeg = m_flightPlan.legs[legIndex];
    leg.legIndex = legIndex;
    leg.waypointIdentifier = planLeg.waypointIdentifier;
    leg.totalDistanceNm = planLeg.distanceFromOrigin;

    double startDist = legIndex == 0 ? 0.0 : m_flightPlan.legs[legIndex - 1].cumulativeDistance;
    double endDist = planLeg.cumulativeDistance;

    int numSteps = std::max(1, std::min(MAX_STEPS_PER_LEG,
        static_cast<int>(std::ceil((endDist - startDist) / m_integrationStepNm))));
    double actualStep = (endDist - startDist) / numSteps;

    IntegrationStep current;
    if (legIndex == 0) {
        current = createInitialStep();
        leg.initialAltitudeFt = current.altitudeFt;
        leg.initialTasKT = current.tasKT;
        leg.initialMach = current.mach;
        leg.initialMassKg = current.massKg;
    } else {
        current = m_result.allSteps.back();
        leg.initialAltitudeFt = current.altitudeFt;
        leg.initialTasKT = current.tasKT;
        leg.initialMach = current.mach;
        leg.initialMassKg = current.massKg;
    }

    leg.finalAltitudeFt = current.altitudeFt;
    leg.finalTasKT = current.tasKT;
    leg.finalMach = current.mach;
    leg.finalMassKg = current.massKg;

    double cumTimeStart = current.elapsedTimeMin;
    double fuelStart = current.fuelRemainingKg;

    double sumGs = 0.0;
    double sumVspd = 0.0;
    double sumHw = 0.0;
    double sumCw = 0.0;
    double sumFuelFlow = 0.0;
    double maxVspd = -1e9;
    double minVspd = 1e9;
    double maxHw = -1e9;
    double peakFuelFlow = 0.0;

    for (int step = 1; step <= numSteps; ++step) {
        double nextDist = startDist + actualStep * step;
        if (nextDist > endDist) nextDist = endDist;

        integrateStep(current, nextDist);

        leg.steps.push_back(current);
        m_result.allSteps.push_back(current);

        leg.finalAltitudeFt = current.altitudeFt;
        leg.finalTasKT = current.tasKT;
        leg.finalMach = current.mach;
        leg.finalMassKg = current.massKg;

        sumGs += current.gsKT;
        sumVspd += current.verticalSpeedFpm;
        sumHw += current.headwindKT;
        sumCw += current.crosswindKT;
        sumFuelFlow += current.fuelFlowKgMin;
        maxVspd = std::max(maxVspd, current.verticalSpeedFpm);
        minVspd = std::min(minVspd, current.verticalSpeedFpm);
        maxHw = std::max(maxHw, current.headwindKT);
        peakFuelFlow = std::max(peakFuelFlow, current.fuelFlowKgMin);

        if (m_result.allSteps.size() >= MAX_TOTAL_STEPS) {
            std::ostringstream oss;
            oss << "达到最大积分步数限制 " << MAX_TOTAL_STEPS
                << "，在距离 " << std::fixed << std::setprecision(1)
                << nextDist << "NM 处提前终止。";
            m_result.warnings.push_back(oss.str());
            break;
        }
    }

    leg.cumulativeTimeMin = current.elapsedTimeMin;
    leg.timeToWaypointMin = current.elapsedTimeMin - cumTimeStart;
    leg.fuelRemainingKg = current.fuelRemainingKg;
    leg.fuelBurnedKg = fuelStart - current.fuelRemainingKg;

    int n = static_cast<int>(leg.steps.size());
    if (n > 0) {
        leg.avgGsKT = sumGs / n;
        leg.avgVerticalSpeedFpm = sumVspd / n;
        leg.maxVerticalSpeedFpm = maxVspd;
        leg.minVerticalSpeedFpm = minVspd;
        leg.avgHeadwindKT = sumHw / n;
        leg.maxHeadwindKT = maxHw;
        leg.avgCrosswindKT = sumCw / n;
        leg.avgFuelFlowKgMin = sumFuelFlow / n;
        leg.peakFuelFlowKgMin = peakFuelFlow;
        leg.driftAngleDeg = leg.steps.back().driftAngleDeg;

        if (leg.totalDistanceNm > 0.0) {
            leg.fuelEfficiencyKgPerNm = leg.fuelBurnedKg / leg.totalDistanceNm;
        }
        if (leg.fuelBurnedKg > 0.0) {
            leg.fuelEfficiencyNmPerKg = leg.totalDistanceNm / leg.fuelBurnedKg;
        }
    }
}

void BADATrajectoryIntegrator::finalizeSummary() {
    if (m_result.allSteps.empty()) return;

    const auto& first = m_result.allSteps.front();
    const auto& last = m_result.allSteps.back();

    m_result.totalTimeMin = last.elapsedTimeMin;
    m_result.totalFuelBurnKg = first.fuelRemainingKg - last.fuelRemainingKg;
    m_result.totalDistanceNm = last.distanceNm;
    m_result.initialFuelKg = first.fuelRemainingKg;
    m_result.finalFuelKg = last.fuelRemainingKg;
    m_result.initialMassKg = first.massKg;
    m_result.finalMassKg = last.massKg;

    double sumGs = 0.0;
    double sumTas = 0.0;
    double sumFuelFlow = 0.0;
    double maxTas = 0.0;
    double maxMach = 0.0;
    double maxGs = 0.0;

    for (const auto& s : m_result.allSteps) {
        sumGs += s.gsKT;
        sumTas += s.tasKT;
        sumFuelFlow += s.fuelFlowKgMin;
        maxTas = std::max(maxTas, s.tasKT);
        maxMach = std::max(maxMach, s.mach);
        maxGs = std::max(maxGs, s.gsKT);
    }

    size_t n = m_result.allSteps.size();
    if (n > 0) {
        m_result.avgGsKT = sumGs / n;
        m_result.avgTasKT = sumTas / n;
        m_result.avgFuelFlowKgMin = sumFuelFlow / n;
    }

    m_result.maxTasKT = maxTas;
    m_result.maxMach = maxMach;
    m_result.maxGsKT = maxGs;

    if (m_result.totalFuelBurnKg > 0.0) {
        m_result.rangeNmPerKg = m_result.totalDistanceNm / m_result.totalFuelBurnKg;
    }

    if (!m_vnavProfile.waypoints.empty()) {
        m_result.topOfClimbDistanceNm = m_vnavProfile.topOfClimbDistanceNm;
        m_result.topOfDescentDistanceNm = m_vnavProfile.topOfDescentDistanceNm;
        m_result.cruiseAltitudeFt = m_vnavProfile.cruiseAltitudeFt;
    } else if (!m_flightPlan.legs.empty()) {
        m_result.topOfClimbDistanceNm = m_flightPlan.totalDistanceNm * 0.25;
        m_result.topOfDescentDistanceNm = m_flightPlan.totalDistanceNm * 0.75;
        m_result.cruiseAltitudeFt = 36000.0;
    }
}

TrajectoryIntegrationResult BADATrajectoryIntegrator::integrate() {
    m_result = TrajectoryIntegrationResult();

    if (m_flightPlan.legs.size() < 2) {
        m_result.success = false;
        m_result.errorMessage = "飞行计划至少需要2个航点";
        return m_result;
    }

    if (m_initialMassKg <= m_aircraft.limits.operatingEmptyWeightKg) {
        m_result.warnings.push_back("初始质量低于操作空重，已自动校正");
        m_initialMassKg = m_aircraft.limits.operatingEmptyWeightKg + m_initialFuelKg;
    }

    if (m_initialFuelKg <= 0.0) {
        m_result.warnings.push_back("初始燃油为0，已设为最小燃油 5000kg");
        m_initialFuelKg = 5000.0;
    }

    if (m_wind.numLayers() == 0) {
        m_wind.setNoWind();
        m_result.warnings.push_back("未设置风场，已自动设为无风");
    }

    try {
        m_result.legs.reserve(m_flightPlan.legs.size());

        for (size_t i = 0; i < m_flightPlan.legs.size(); ++i) {
            LegIntegrationResult leg;
            integrateLeg(i, leg);
            m_result.legs.push_back(leg);

            if (!m_result.allSteps.empty() &&
                m_result.allSteps.back().fuelRemainingKg <= 0.0) {
                break;
            }
        }

        finalizeSummary();

        if (m_result.finalFuelKg <= 0.0 && !m_result.warnings.empty()) {
            m_result.errorMessage = "飞行计划不可行：到达目的地前燃油耗尽";
            m_result.success = false;
        } else {
            m_result.success = true;
        }

    } catch (const std::exception& e) {
        m_result.success = false;
        m_result.errorMessage = std::string("轨迹积分器异常: ") + e.what();
    } catch (...) {
        m_result.success = false;
        m_result.errorMessage = "轨迹积分器发生未知错误";
    }

    return m_result;
}

}
}
