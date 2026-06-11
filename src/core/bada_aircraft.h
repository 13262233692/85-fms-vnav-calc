#pragma once

#include "bada_atmosphere.h"
#include <string>
#include <vector>
#include <array>
#include <cmath>

namespace fmc {
namespace bada {

enum class AircraftType {
    A320,
    B737_800,
    B777_300ER,
    A330_300,
    B787_9,
    CUSTOM
};

enum class FlightPhase {
    Takeoff,
    Climb,
    Cruise,
    Descent,
    Approach,
    Landing
};

enum class EngineType {
    Jet,
    Turboprop,
    Piston
};

struct BADAEngineData {
    EngineType type = EngineType::Jet;
    double maxThrustSeaLevelN = 0.0;
    double climbThrustCoeff1 = 0.0;
    double climbThrustCoeff2 = 0.0;
    double cruiseThrustCoeff1 = 0.0;
    double cruiseThrustCoeff2 = 0.0;
    double descentThrustCoeff1 = 0.0;
    double descentThrustCoeff2 = 0.0;
    double approachIdleThrustN = 0.0;
    double fuelFlowCoeff1 = 0.0;
    double fuelFlowCoeff2 = 0.0;
    double fuelFlowCoeff3 = 0.0;
    double fuelFlowIdle = 0.0;
};

struct BADAAerodynamicsData {
    double referenceMassKg = 0.0;
    double minimumMassKg = 0.0;
    double maximumMassKg = 0.0;
    double wingAreaM2 = 0.0;
    double wingSpanM = 0.0;
    double aspectRatio = 0.0;
    double oswaldEfficiency = 0.8;

    double cd0Low = 0.0;
    double cd0High = 0.0;
    double cd2Low = 0.0;
    double cd2High = 0.0;
    double machDragDivergence = 0.75;
    double compressibilityDragCoeff = 0.0;

    double gearDragCoeff = 0.0;
    double flapDragCoeffTakeoff = 0.0;
    double flapDragCoeffApproach = 0.0;
    double flapDragCoeffLanding = 0.0;

    double clMaxTakeoff = 1.8;
    double clMaxApproach = 2.0;
    double clMaxLanding = 2.4;
    double clMaxClean = 1.4;

    double stallSpeedCleanKcas = 130.0;
    double stallSpeedApproachKcas = 110.0;
    double stallSpeedLandingKcas = 105.0;

    std::array<double, 6> dragCoeff = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
};

struct BADAOperatingLimits {
    double maxMach = 0.82;
    double maxOperatingAltitudeFt = 41000.0;
    double maxSpeedKcasBelow10kFt = 250.0;
    double normalClimbRateFpm = 1800.0;
    double maxClimbRateFpm = 3000.0;
    double normalDescentRateFpm = 2000.0;
    double maxDescentRateFpm = 3500.0;
    double vmoKcas = 350.0;
    double cruiseAltitudeFt = 36000.0;
    double initialCruiseWeightKg = 0.0;
    double operatingEmptyWeightKg = 0.0;
    double maximumPayloadKg = 0.0;
    double maximumFuelKg = 0.0;
};

struct BADAPerformanceData {
    double standardClimbSpeedScheduleKcas = 300.0;
    double standardClimbMach = 0.78;
    double standardCruiseMach = 0.8;
    double standardDescentSpeedScheduleKcas = 320.0;
    double standardDescentMach = 0.78;
    double transitionAltitudeFt = 10000.0;
    double crossOverAltitudeFt = 28000.0;
    double costIndex = 30.0;
};

struct BADAAircraft {
    AircraftType type = AircraftType::A320;
    std::string modelName;
    std::string manufacturer;
    std::string registration;
    int numEngines = 2;

    BADAEngineData engine;
    BADAAerodynamicsData aerodynamics;
    BADAOperatingLimits limits;
    BADAPerformanceData performance;

    static BADAAircraft airbusA320Neo();
    static BADAAircraft boeing737Max8();
    static BADAAircraft boeing777_300ER();
    static BADAAircraft airbusA330_300();
    static BADAAircraft boeing787_9();

    std::string getTypeString() const;
};

class BADAAerodynamics {
public:
    static double calculateLiftCoefficient(const BADAAircraft& ac,
                                            double tasKT,
                                            double altitudeFt,
                                            double massKg,
                                            double bankAngleDeg = 0.0);

    static double calculateZeroLiftDrag(const BADAAircraft& ac,
                                         double mach,
                                         FlightPhase phase,
                                         double machDragDivergence = 0.0);

    static double calculateInducedDrag(const BADAAircraft& ac,
                                        double liftCoefficient);

    static double calculateTotalDrag(const BADAAircraft& ac,
                                      double tasKT,
                                      double altitudeFt,
                                      double massKg,
                                      FlightPhase phase,
                                      double bankAngleDeg = 0.0,
                                      double gearDown = false,
                                      double flapsTakeoff = false,
                                      double flapsApproach = false);

    static double calculateDragPolar(const BADAAircraft& ac, double liftCoefficient);

    static double calculateMaximumThrust(const BADAAircraft& ac,
                                          double altitudeFt,
                                          FlightPhase phase);

    static double calculateFuelFlowKgMin(const BADAAircraft& ac,
                                          double thrustN,
                                          double altitudeFt,
                                          double mach,
                                          FlightPhase phase);

    static double calculateRangeKgPerNm(const BADAAircraft& ac,
                                         double tasKT,
                                         double altitudeFt,
                                         double massKg,
                                         FlightPhase phase);

    static double calculateStallSpeedKcas(const BADAAircraft& ac,
                                           double massKg,
                                           FlightPhase phase);
};

}
}
