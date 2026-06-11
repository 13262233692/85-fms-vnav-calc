#pragma once

#include "bada_atmosphere.h"
#include "bada_aircraft.h"
#include "bada_wind.h"
#include "navigation.h"
#include "vnav_solver.h"

#include <vector>
#include <cmath>
#include <algorithm>

namespace fmc {
namespace bada {

struct IntegrationStep {
    double distanceNm = 0.0;
    double altitudeFt = 0.0;
    double tasKT = 0.0;
    double mach = 0.0;
    double gsKT = 0.0;
    double trueTrackDeg = 0.0;
    double driftAngleDeg = 0.0;
    double headwindKT = 0.0;
    double crosswindKT = 0.0;
    double massKg = 0.0;
    double fuelRemainingKg = 0.0;
    double fuelFlowKgMin = 0.0;
    double thrustN = 0.0;
    double dragN = 0.0;
    double liftCoefficient = 0.0;
    double verticalSpeedFpm = 0.0;
    double flightPathAngleDeg = 0.0;
    double elapsedTimeMin = 0.0;
    double temperatureOffsetK = 0.0;
    AtmosphericState atmosphere;
    FlightPhase phase = FlightPhase::Cruise;
};

struct LegIntegrationResult {
    size_t legIndex;
    std::string waypointIdentifier;
    double totalDistanceNm;
    double initialAltitudeFt;
    double finalAltitudeFt;
    double initialTasKT;
    double finalTasKT;
    double avgGsKT;
    double initialMach;
    double finalMach;
    double timeToWaypointMin;
    double cumulativeTimeMin;
    double fuelBurnedKg;
    double fuelRemainingKg;
    double fuelEfficiencyKgPerNm;
    double fuelEfficiencyNmPerKg;
    double avgFuelFlowKgMin;
    double peakFuelFlowKgMin;
    double initialMassKg;
    double finalMassKg;
    double avgVerticalSpeedFpm;
    double maxVerticalSpeedFpm;
    double minVerticalSpeedFpm;
    double avgHeadwindKT;
    double maxHeadwindKT;
    double avgCrosswindKT;
    double driftAngleDeg;

    std::vector<IntegrationStep> steps;
};

struct TrajectoryIntegrationResult {
    bool success = false;
    std::string errorMessage;
    double totalTimeMin;
    double totalFuelBurnKg;
    double totalDistanceNm;
    double initialFuelKg;
    double finalFuelKg;
    double initialMassKg;
    double finalMassKg;
    double avgGsKT;
    double avgTasKT;
    double avgFuelFlowKgMin;
    double rangeNmPerKg;

    double topOfClimbDistanceNm;
    double topOfDescentDistanceNm;
    double cruiseAltitudeFt;
    double maxTasKT;
    double maxMach;
    double maxGsKT;

    std::vector<LegIntegrationResult> legs;
    std::vector<IntegrationStep> allSteps;
    std::vector<std::string> warnings;
};

class BADATrajectoryIntegrator {
public:
    BADATrajectoryIntegrator();

    void setAircraft(const BADAAircraft& aircraft);
    void setWindModel(const WindModel& wind);
    void setFlightPlan(const nav::FlightPlan& flightPlan);
    void setVNAVProfile(const nav::VNAVProfile& vnavProfile);
    void setInitialFuelKg(double fuelKg);
    void setInitialMassKg(double massKg);
    void setCruiseMach(double mach);
    void setIntegrationStepNm(double stepNm);
    void setCostIndex(double costIndex);

    TrajectoryIntegrationResult integrate();

    const BADAAircraft& getAircraft() const { return m_aircraft; }
    const WindModel& getWindModel() const { return m_wind; }

    static constexpr double DEFAULT_STEP_NM = 1.0;
    static constexpr int MAX_STEPS_PER_LEG = 500;
    static constexpr int MAX_TOTAL_STEPS = 50000;

private:
    BADAAircraft m_aircraft;
    WindModel m_wind;
    nav::FlightPlan m_flightPlan;
    nav::VNAVProfile m_vnavProfile;
    double m_initialFuelKg;
    double m_initialMassKg;
    double m_cruiseMach;
    double m_integrationStepNm;
    double m_costIndex;

    TrajectoryIntegrationResult m_result;

    double getAltitudeAtDistanceNm(double distanceNm) const;
    FlightPhase getPhaseAtDistanceNm(double distanceNm) const;
    double getTrueTrackAtDistanceNm(double distanceNm) const;
    double calculateTargetTasKT(double altitudeFt, FlightPhase phase, double& outMach) const;
    IntegrationStep createInitialStep();
    void integrateStep(IntegrationStep& current, double nextDistanceNm);
    void integrateLeg(size_t legIndex, LegIntegrationResult& legResult);
    void summarizeLeg(LegIntegrationResult& legResult);
    void finalizeSummary();
    double interpolate(double x0, double y0, double x1, double y1, double x) const;
};

}
}
