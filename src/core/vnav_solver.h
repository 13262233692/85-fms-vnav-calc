#pragma once

#include "navigation.h"
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <limits>
#include <algorithm>
#include <cmath>

namespace fmc {
namespace nav {

enum class AltitudeConstraintType {
    None,
    At,
    AtOrAbove,
    AtOrBelow,
    Between
};

struct AltitudeConstraint {
    AltitudeConstraintType type = AltitudeConstraintType::None;
    double altitudeAtFt = 0.0;
    double altitudeAboveFt = 0.0;
    double altitudeBelowFt = 0.0;
    std::string source;
    bool isHardConstraint = true;

    static AltitudeConstraint makeAt(double altFt, const std::string& src = "") {
        AltitudeConstraint c;
        c.type = AltitudeConstraintType::At;
        c.altitudeAtFt = altFt;
        c.source = src;
        c.isHardConstraint = true;
        return c;
    }
    static AltitudeConstraint makeAbove(double altFt, const std::string& src = "") {
        AltitudeConstraint c;
        c.type = AltitudeConstraintType::AtOrAbove;
        c.altitudeAboveFt = altFt;
        c.source = src;
        c.isHardConstraint = true;
        return c;
    }
    static AltitudeConstraint makeBelow(double altFt, const std::string& src = "") {
        AltitudeConstraint c;
        c.type = AltitudeConstraintType::AtOrBelow;
        c.altitudeBelowFt = altFt;
        c.source = src;
        c.isHardConstraint = true;
        return c;
    }
    static AltitudeConstraint makeBetween(double below, double above, const std::string& src = "") {
        AltitudeConstraint c;
        c.type = AltitudeConstraintType::Between;
        c.altitudeAboveFt = std::min(above, below);
        c.altitudeBelowFt = std::max(above, below);
        c.source = src;
        c.isHardConstraint = true;
        return c;
    }
    double getMinAllowed() const {
        switch (type) {
            case AltitudeConstraintType::At: return altitudeAtFt;
            case AltitudeConstraintType::AtOrAbove: return altitudeAboveFt;
            case AltitudeConstraintType::AtOrBelow: return 0.0;
            case AltitudeConstraintType::Between: return altitudeAboveFt;
            default: return -std::numeric_limits<double>::infinity();
        }
    }
    double getMaxAllowed() const {
        switch (type) {
            case AltitudeConstraintType::At: return altitudeAtFt;
            case AltitudeConstraintType::AtOrAbove: return std::numeric_limits<double>::infinity();
            case AltitudeConstraintType::AtOrBelow: return altitudeBelowFt;
            case AltitudeConstraintType::Between: return altitudeBelowFt;
            default: return std::numeric_limits<double>::infinity();
        }
    }
};

enum class VerticalFlightPhase {
    Unknown,
    Climb,
    Cruise,
    Descent,
    LevelOff
};

struct AircraftPerformance {
    double maxClimbRateFpm = 2500.0;
    double maxDescentRateFpm = 3000.0;
    double normalDescentRateFpm = 1500.0;
    double maxClimbGradientFtPerNm = 500.0;
    double maxDescentGradientFtPerNm = 1200.0;
    double normalDescentGradientFtPerNm = 318.0;
    double cruiseSpeedKtas = 450.0;
    double descentSpeedKtas = 320.0;
    double approachSpeedKtas = 140.0;
    double initialCruiseAltitudeFt = 35000.0;

    static AircraftPerformance boeing737() {
        AircraftPerformance p;
        p.maxClimbRateFpm = 3000.0;
        p.maxDescentRateFpm = 3500.0;
        p.normalDescentRateFpm = 1800.0;
        p.maxClimbGradientFtPerNm = 600.0;
        p.maxDescentGradientFtPerNm = 1500.0;
        p.normalDescentGradientFtPerNm = 318.0;
        p.cruiseSpeedKtas = 470.0;
        p.descentSpeedKtas = 340.0;
        p.approachSpeedKtas = 145.0;
        p.initialCruiseAltitudeFt = 37000.0;
        return p;
    }
    static AircraftPerformance airbusA320() {
        AircraftPerformance p;
        p.maxClimbRateFpm = 2800.0;
        p.maxDescentRateFpm = 3200.0;
        p.normalDescentRateFpm = 1700.0;
        p.maxClimbGradientFtPerNm = 550.0;
        p.maxDescentGradientFtPerNm = 1400.0;
        p.normalDescentGradientFtPerNm = 300.0;
        p.cruiseSpeedKtas = 450.0;
        p.descentSpeedKtas = 330.0;
        p.approachSpeedKtas = 140.0;
        p.initialCruiseAltitudeFt = 36000.0;
        return p;
    }
};

struct VNAVWaypointSolution {
    size_t legIndex;
    std::string waypointIdentifier;
    double cumulativeDistanceNm;
    double distanceFromPreviousNm;
    double minAllowedAltitudeFt;
    double maxAllowedAltitudeFt;
    double plannedAltitudeFt;
    double climbLimitFt;
    double descentLimitFt;
    VerticalFlightPhase phase;
    double verticalSpeedFpm;
    double flightPathAngleDeg;
    double timeToWaypointMin;
    bool constraintViolated;
    std::string violationMessage;
    AltitudeConstraint originalConstraint;
};

enum class VNAVSolverStatus {
    Success,
    SuccessWithWarnings,
    ConstraintInfeasible,
    InvalidInput,
    InternalError
};

struct VNAVProfile {
    VNAVSolverStatus status;
    std::vector<VNAVWaypointSolution> waypoints;
    double totalDistanceNm;
    double topOfClimbDistanceNm;
    double topOfDescentDistanceNm;
    double cruiseAltitudeFt;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
    bool gradientTruncationApplied;
    int numConstraintsRelaxed;

    VNAVProfile()
        : status(VNAVSolverStatus::InvalidInput),
          totalDistanceNm(0.0),
          topOfClimbDistanceNm(0.0),
          topOfDescentDistanceNm(0.0),
          cruiseAltitudeFt(0.0),
          gradientTruncationApplied(false),
          numConstraintsRelaxed(0) {}

    bool isValid() const {
        return status == VNAVSolverStatus::Success ||
               status == VNAVSolverStatus::SuccessWithWarnings;
    }
};

class VNAVSolver {
public:
    VNAVSolver();

    void setAircraftPerformance(const AircraftPerformance& perf);
    void setFlightPlan(const FlightPlan& plan);
    void setAltitudeConstraints(const std::vector<AltitudeConstraint>& constraints);
    void setCruiseAltitude(double cruiseAltFt);

    VNAVProfile solve();

    const AircraftPerformance& getPerformance() const { return m_performance; }
    const FlightPlan& getFlightPlan() const { return m_flightPlan; }

    static constexpr double TOLERANCE_FT = 10.0;
    static constexpr int MAX_ITERATIONS = 100;

private:
    AircraftPerformance m_performance;
    FlightPlan m_flightPlan;
    std::vector<AltitudeConstraint> m_constraints;
    double m_cruiseAltitudeFt;
    VNAVProfile m_result;

    void initializeWaypoints();
    void applyClimbLimitsForward();
    void applyDescentLimitsBackward();
    void enforceConstraintBounds();
    void computePlannedAltitudes();
    void detectPhaseTransitions();
    void checkFeasibilityAndRelax();
    void computeVerticalParameters();

    double safeClimbFrom(double prevAlt, double distanceNm) const;
    double safeDescentFrom(double prevAlt, double distanceNm) const;
    double safeClimbTo(double nextAlt, double distanceNm) const;
    double safeDescentTo(double nextAlt, double distanceNm) const;
    double gradientLimitClimb(double distanceNm) const;
    double gradientLimitDescent(double distanceNm) const;
    void clamp(double& value, double lo, double hi) const;
};

}
}
