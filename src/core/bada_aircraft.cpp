#include "bada_aircraft.h"
#include "bada_wind.h"
#include <cmath>
#include <algorithm>

namespace fmc {
namespace bada {

BADAAircraft BADAAircraft::airbusA320Neo() {
    BADAAircraft ac;
    ac.type = AircraftType::A320;
    ac.modelName = "A320-251N Neo";
    ac.manufacturer = "Airbus";
    ac.registration = "F-WWBA";
    ac.numEngines = 2;

    ac.engine.type = EngineType::Jet;
    ac.engine.maxThrustSeaLevelN = 120400.0;
    ac.engine.climbThrustCoeff1 = 0.85;
    ac.engine.climbThrustCoeff2 = 0.00002;
    ac.engine.cruiseThrustCoeff1 = 0.07;
    ac.engine.cruiseThrustCoeff2 = 0.0000006;
    ac.engine.descentThrustCoeff1 = 0.05;
    ac.engine.descentThrustCoeff2 = 0.0;
    ac.engine.approachIdleThrustN = 3000.0;
    ac.engine.fuelFlowCoeff1 = 0.000018;
    ac.engine.fuelFlowCoeff2 = 0.7;
    ac.engine.fuelFlowCoeff3 = 0.000002;
    ac.engine.fuelFlowIdle = 12.0;

    ac.aerodynamics.referenceMassKg = 68000.0;
    ac.aerodynamics.minimumMassKg = 40000.0;
    ac.aerodynamics.maximumMassKg = 79000.0;
    ac.aerodynamics.wingAreaM2 = 124.0;
    ac.aerodynamics.wingSpanM = 35.8;
    ac.aerodynamics.aspectRatio = 10.3;
    ac.aerodynamics.oswaldEfficiency = 0.78;

    ac.aerodynamics.cd0Low = 0.020;
    ac.aerodynamics.cd0High = 0.028;
    ac.aerodynamics.cd2Low = 0.045;
    ac.aerodynamics.cd2High = 0.065;
    ac.aerodynamics.machDragDivergence = 0.82;
    ac.aerodynamics.compressibilityDragCoeff = 25.0;

    ac.aerodynamics.gearDragCoeff = 0.02;
    ac.aerodynamics.flapDragCoeffTakeoff = 0.01;
    ac.aerodynamics.flapDragCoeffApproach = 0.02;
    ac.aerodynamics.flapDragCoeffLanding = 0.04;

    ac.aerodynamics.clMaxTakeoff = 1.85;
    ac.aerodynamics.clMaxApproach = 2.1;
    ac.aerodynamics.clMaxLanding = 2.5;
    ac.aerodynamics.clMaxClean = 1.5;

    ac.aerodynamics.stallSpeedCleanKcas = 132.0;
    ac.aerodynamics.stallSpeedApproachKcas = 112.0;
    ac.aerodynamics.stallSpeedLandingKcas = 107.0;

    ac.aerodynamics.dragCoeff = {{0.020, 0.0, 0.045, 0.0, 0.0, 0.0}};

    ac.limits.maxMach = 0.82;
    ac.limits.maxOperatingAltitudeFt = 39800.0;
    ac.limits.maxSpeedKcasBelow10kFt = 250.0;
    ac.limits.normalClimbRateFpm = 1800.0;
    ac.limits.maxClimbRateFpm = 3200.0;
    ac.limits.normalDescentRateFpm = 2000.0;
    ac.limits.maxDescentRateFpm = 3800.0;
    ac.limits.vmoKcas = 350.0;
    ac.limits.cruiseAltitudeFt = 36000.0;
    ac.limits.initialCruiseWeightKg = 65000.0;
    ac.limits.operatingEmptyWeightKg = 40800.0;
    ac.limits.maximumPayloadKg = 19200.0;
    ac.limits.maximumFuelKg = 23800.0;

    ac.performance.standardClimbSpeedScheduleKcas = 300.0;
    ac.performance.standardClimbMach = 0.78;
    ac.performance.standardCruiseMach = 0.785;
    ac.performance.standardDescentSpeedScheduleKcas = 320.0;
    ac.performance.standardDescentMach = 0.78;
    ac.performance.transitionAltitudeFt = 10000.0;
    ac.performance.crossOverAltitudeFt = 29000.0;
    ac.performance.costIndex = 30.0;

    return ac;
}

BADAAircraft BADAAircraft::boeing737Max8() {
    BADAAircraft ac;
    ac.type = AircraftType::B737_800;
    ac.modelName = "737-MAX 8";
    ac.manufacturer = "Boeing";
    ac.registration = "N8701Q";
    ac.numEngines = 2;

    ac.engine.type = EngineType::Jet;
    ac.engine.maxThrustSeaLevelN = 119900.0;
    ac.engine.climbThrustCoeff1 = 0.86;
    ac.engine.climbThrustCoeff2 = 0.000018;
    ac.engine.cruiseThrustCoeff1 = 0.068;
    ac.engine.cruiseThrustCoeff2 = 0.00000055;
    ac.engine.descentThrustCoeff1 = 0.045;
    ac.engine.descentThrustCoeff2 = 0.0;
    ac.engine.approachIdleThrustN = 2900.0;
    ac.engine.fuelFlowCoeff1 = 0.0000175;
    ac.engine.fuelFlowCoeff2 = 0.72;
    ac.engine.fuelFlowCoeff3 = 0.0000019;
    ac.engine.fuelFlowIdle = 11.5;

    ac.aerodynamics.referenceMassKg = 65771.0;
    ac.aerodynamics.minimumMassKg = 40000.0;
    ac.aerodynamics.maximumMassKg = 82191.0;
    ac.aerodynamics.wingAreaM2 = 127.0;
    ac.aerodynamics.wingSpanM = 35.92;
    ac.aerodynamics.aspectRatio = 9.72;
    ac.aerodynamics.oswaldEfficiency = 0.75;

    ac.aerodynamics.cd0Low = 0.021;
    ac.aerodynamics.cd0High = 0.029;
    ac.aerodynamics.cd2Low = 0.042;
    ac.aerodynamics.cd2High = 0.062;
    ac.aerodynamics.machDragDivergence = 0.82;
    ac.aerodynamics.compressibilityDragCoeff = 23.0;

    ac.aerodynamics.gearDragCoeff = 0.022;
    ac.aerodynamics.flapDragCoeffTakeoff = 0.011;
    ac.aerodynamics.flapDragCoeffApproach = 0.021;
    ac.aerodynamics.flapDragCoeffLanding = 0.042;

    ac.aerodynamics.clMaxTakeoff = 1.82;
    ac.aerodynamics.clMaxApproach = 2.08;
    ac.aerodynamics.clMaxLanding = 2.48;
    ac.aerodynamics.clMaxClean = 1.48;

    ac.aerodynamics.stallSpeedCleanKcas = 135.0;
    ac.aerodynamics.stallSpeedApproachKcas = 115.0;
    ac.aerodynamics.stallSpeedLandingKcas = 110.0;

    ac.aerodynamics.dragCoeff = {{0.021, 0.0, 0.042, 0.0, 0.0, 0.0}};

    ac.limits.maxMach = 0.82;
    ac.limits.maxOperatingAltitudeFt = 41000.0;
    ac.limits.maxSpeedKcasBelow10kFt = 250.0;
    ac.limits.normalClimbRateFpm = 1900.0;
    ac.limits.maxClimbRateFpm = 3400.0;
    ac.limits.normalDescentRateFpm = 2100.0;
    ac.limits.maxDescentRateFpm = 4000.0;
    ac.limits.vmoKcas = 340.0;
    ac.limits.cruiseAltitudeFt = 37000.0;
    ac.limits.initialCruiseWeightKg = 68000.0;
    ac.limits.operatingEmptyWeightKg = 41413.0;
    ac.limits.maximumPayloadKg = 20883.0;
    ac.limits.maximumFuelKg = 25849.0;

    ac.performance.standardClimbSpeedScheduleKcas = 300.0;
    ac.performance.standardClimbMach = 0.785;
    ac.performance.standardCruiseMach = 0.79;
    ac.performance.standardDescentSpeedScheduleKcas = 320.0;
    ac.performance.standardDescentMach = 0.785;
    ac.performance.transitionAltitudeFt = 10000.0;
    ac.performance.crossOverAltitudeFt = 28500.0;
    ac.performance.costIndex = 30.0;

    return ac;
}

BADAAircraft BADAAircraft::boeing777_300ER() {
    BADAAircraft ac;
    ac.type = AircraftType::B777_300ER;
    ac.modelName = "777-36NER";
    ac.manufacturer = "Boeing";
    ac.registration = "B-2088";
    ac.numEngines = 2;

    ac.engine.type = EngineType::Jet;
    ac.engine.maxThrustSeaLevelN = 489300.0;
    ac.engine.climbThrustCoeff1 = 0.90;
    ac.engine.climbThrustCoeff2 = 0.000015;
    ac.engine.cruiseThrustCoeff1 = 0.065;
    ac.engine.cruiseThrustCoeff2 = 0.0000005;
    ac.engine.descentThrustCoeff1 = 0.04;
    ac.engine.descentThrustCoeff2 = 0.0;
    ac.engine.approachIdleThrustN = 8000.0;
    ac.engine.fuelFlowCoeff1 = 0.000009;
    ac.engine.fuelFlowCoeff2 = 0.65;
    ac.engine.fuelFlowCoeff3 = 0.0000015;
    ac.engine.fuelFlowIdle = 22.0;

    ac.aerodynamics.referenceMassKg = 262000.0;
    ac.aerodynamics.minimumMassKg = 150000.0;
    ac.aerodynamics.maximumMassKg = 351533.0;
    ac.aerodynamics.wingAreaM2 = 436.8;
    ac.aerodynamics.wingSpanM = 64.8;
    ac.aerodynamics.aspectRatio = 9.6;
    ac.aerodynamics.oswaldEfficiency = 0.76;

    ac.aerodynamics.cd0Low = 0.017;
    ac.aerodynamics.cd0High = 0.025;
    ac.aerodynamics.cd2Low = 0.038;
    ac.aerodynamics.cd2High = 0.058;
    ac.aerodynamics.machDragDivergence = 0.86;
    ac.aerodynamics.compressibilityDragCoeff = 30.0;

    ac.aerodynamics.gearDragCoeff = 0.018;
    ac.aerodynamics.flapDragCoeffTakeoff = 0.009;
    ac.aerodynamics.flapDragCoeffApproach = 0.018;
    ac.aerodynamics.flapDragCoeffLanding = 0.038;

    ac.aerodynamics.clMaxTakeoff = 1.90;
    ac.aerodynamics.clMaxApproach = 2.15;
    ac.aerodynamics.clMaxLanding = 2.55;
    ac.aerodynamics.clMaxClean = 1.55;

    ac.aerodynamics.stallSpeedCleanKcas = 155.0;
    ac.aerodynamics.stallSpeedApproachKcas = 130.0;
    ac.aerodynamics.stallSpeedLandingKcas = 125.0;

    ac.aerodynamics.dragCoeff = {{0.017, 0.0, 0.038, 0.0, 0.0, 0.0}};

    ac.limits.maxMach = 0.87;
    ac.limits.maxOperatingAltitudeFt = 43100.0;
    ac.limits.maxSpeedKcasBelow10kFt = 250.0;
    ac.limits.normalClimbRateFpm = 2200.0;
    ac.limits.maxClimbRateFpm = 3800.0;
    ac.limits.normalDescentRateFpm = 2400.0;
    ac.limits.maxDescentRateFpm = 4500.0;
    ac.limits.vmoKcas = 360.0;
    ac.limits.cruiseAltitudeFt = 39000.0;
    ac.limits.initialCruiseWeightKg = 300000.0;
    ac.limits.operatingEmptyWeightKg = 167829.0;
    ac.limits.maximumPayloadKg = 71827.0;
    ac.limits.maximumFuelKg = 145520.0;

    ac.performance.standardClimbSpeedScheduleKcas = 320.0;
    ac.performance.standardClimbMach = 0.84;
    ac.performance.standardCruiseMach = 0.84;
    ac.performance.standardDescentSpeedScheduleKcas = 340.0;
    ac.performance.standardDescentMach = 0.84;
    ac.performance.transitionAltitudeFt = 10000.0;
    ac.performance.crossOverAltitudeFt = 30000.0;
    ac.performance.costIndex = 40.0;

    return ac;
}

BADAAircraft BADAAircraft::airbusA330_300() {
    BADAAircraft ac;
    ac.type = AircraftType::A330_300;
    ac.modelName = "A330-343";
    ac.manufacturer = "Airbus";
    ac.registration = "F-WWKA";
    ac.numEngines = 2;

    ac.engine.type = EngineType::Jet;
    ac.engine.maxThrustSeaLevelN = 308000.0;
    ac.engine.climbThrustCoeff1 = 0.88;
    ac.engine.climbThrustCoeff2 = 0.000016;
    ac.engine.cruiseThrustCoeff1 = 0.066;
    ac.engine.cruiseThrustCoeff2 = 0.00000052;
    ac.engine.descentThrustCoeff1 = 0.042;
    ac.engine.descentThrustCoeff2 = 0.0;
    ac.engine.approachIdleThrustN = 5500.0;
    ac.engine.fuelFlowCoeff1 = 0.000011;
    ac.engine.fuelFlowCoeff2 = 0.67;
    ac.engine.fuelFlowCoeff3 = 0.0000016;
    ac.engine.fuelFlowIdle = 18.0;

    ac.aerodynamics.referenceMassKg = 165000.0;
    ac.aerodynamics.minimumMassKg = 100000.0;
    ac.aerodynamics.maximumMassKg = 242000.0;
    ac.aerodynamics.wingAreaM2 = 361.6;
    ac.aerodynamics.wingSpanM = 60.3;
    ac.aerodynamics.aspectRatio = 10.06;
    ac.aerodynamics.oswaldEfficiency = 0.77;

    ac.aerodynamics.cd0Low = 0.018;
    ac.aerodynamics.cd0High = 0.026;
    ac.aerodynamics.cd2Low = 0.040;
    ac.aerodynamics.cd2High = 0.060;
    ac.aerodynamics.machDragDivergence = 0.84;
    ac.aerodynamics.compressibilityDragCoeff = 27.0;

    ac.aerodynamics.gearDragCoeff = 0.019;
    ac.aerodynamics.flapDragCoeffTakeoff = 0.0095;
    ac.aerodynamics.flapDragCoeffApproach = 0.019;
    ac.aerodynamics.flapDragCoeffLanding = 0.039;

    ac.aerodynamics.clMaxTakeoff = 1.88;
    ac.aerodynamics.clMaxApproach = 2.13;
    ac.aerodynamics.clMaxLanding = 2.53;
    ac.aerodynamics.clMaxClean = 1.52;

    ac.aerodynamics.stallSpeedCleanKcas = 145.0;
    ac.aerodynamics.stallSpeedApproachKcas = 125.0;
    ac.aerodynamics.stallSpeedLandingKcas = 120.0;

    ac.aerodynamics.dragCoeff = {{0.018, 0.0, 0.040, 0.0, 0.0, 0.0}};

    ac.limits.maxMach = 0.86;
    ac.limits.maxOperatingAltitudeFt = 41000.0;
    ac.limits.maxSpeedKcasBelow10kFt = 250.0;
    ac.limits.normalClimbRateFpm = 2000.0;
    ac.limits.maxClimbRateFpm = 3500.0;
    ac.limits.normalDescentRateFpm = 2200.0;
    ac.limits.maxDescentRateFpm = 4200.0;
    ac.limits.vmoKcas = 355.0;
    ac.limits.cruiseAltitudeFt = 38000.0;
    ac.limits.initialCruiseWeightKg = 210000.0;
    ac.limits.operatingEmptyWeightKg = 107000.0;
    ac.limits.maximumPayloadKg = 45600.0;
    ac.limits.maximumFuelKg = 97530.0;

    ac.performance.standardClimbSpeedScheduleKcas = 310.0;
    ac.performance.standardClimbMach = 0.82;
    ac.performance.standardCruiseMach = 0.82;
    ac.performance.standardDescentSpeedScheduleKcas = 330.0;
    ac.performance.standardDescentMach = 0.82;
    ac.performance.transitionAltitudeFt = 10000.0;
    ac.performance.crossOverAltitudeFt = 29500.0;
    ac.performance.costIndex = 35.0;

    return ac;
}

BADAAircraft BADAAircraft::boeing787_9() {
    BADAAircraft ac;
    ac.type = AircraftType::B787_9;
    ac.modelName = "787-9 Dreamliner";
    ac.manufacturer = "Boeing";
    ac.registration = "N789BA";
    ac.numEngines = 2;

    ac.engine.type = EngineType::Jet;
    ac.engine.maxThrustSeaLevelN = 318000.0;
    ac.engine.climbThrustCoeff1 = 0.89;
    ac.engine.climbThrustCoeff2 = 0.000016;
    ac.engine.cruiseThrustCoeff1 = 0.063;
    ac.engine.cruiseThrustCoeff2 = 0.00000048;
    ac.engine.descentThrustCoeff1 = 0.038;
    ac.engine.descentThrustCoeff2 = 0.0;
    ac.engine.approachIdleThrustN = 5200.0;
    ac.engine.fuelFlowCoeff1 = 0.000010;
    ac.engine.fuelFlowCoeff2 = 0.63;
    ac.engine.fuelFlowCoeff3 = 0.0000014;
    ac.engine.fuelFlowIdle = 16.5;

    ac.aerodynamics.referenceMassKg = 181000.0;
    ac.aerodynamics.minimumMassKg = 110000.0;
    ac.aerodynamics.maximumMassKg = 254000.0;
    ac.aerodynamics.wingAreaM2 = 377.0;
    ac.aerodynamics.wingSpanM = 60.12;
    ac.aerodynamics.aspectRatio = 9.6;
    ac.aerodynamics.oswaldEfficiency = 0.80;

    ac.aerodynamics.cd0Low = 0.016;
    ac.aerodynamics.cd0High = 0.024;
    ac.aerodynamics.cd2Low = 0.037;
    ac.aerodynamics.cd2High = 0.057;
    ac.aerodynamics.machDragDivergence = 0.87;
    ac.aerodynamics.compressibilityDragCoeff = 32.0;

    ac.aerodynamics.gearDragCoeff = 0.017;
    ac.aerodynamics.flapDragCoeffTakeoff = 0.0085;
    ac.aerodynamics.flapDragCoeffApproach = 0.017;
    ac.aerodynamics.flapDragCoeffLanding = 0.037;

    ac.aerodynamics.clMaxTakeoff = 1.92;
    ac.aerodynamics.clMaxApproach = 2.17;
    ac.aerodynamics.clMaxLanding = 2.57;
    ac.aerodynamics.clMaxClean = 1.57;

    ac.aerodynamics.stallSpeedCleanKcas = 148.0;
    ac.aerodynamics.stallSpeedApproachKcas = 127.0;
    ac.aerodynamics.stallSpeedLandingKcas = 122.0;

    ac.aerodynamics.dragCoeff = {{0.016, 0.0, 0.037, 0.0, 0.0, 0.0}};

    ac.limits.maxMach = 0.89;
    ac.limits.maxOperatingAltitudeFt = 43000.0;
    ac.limits.maxSpeedKcasBelow10kFt = 250.0;
    ac.limits.normalClimbRateFpm = 2100.0;
    ac.limits.maxClimbRateFpm = 3600.0;
    ac.limits.normalDescentRateFpm = 2300.0;
    ac.limits.maxDescentRateFpm = 4300.0;
    ac.limits.vmoKcas = 0.0;
    ac.limits.cruiseAltitudeFt = 40000.0;
    ac.limits.initialCruiseWeightKg = 220000.0;
    ac.limits.operatingEmptyWeightKg = 119950.0;
    ac.limits.maximumPayloadKg = 51300.0;
    ac.limits.maximumFuelKg = 101500.0;

    ac.performance.standardClimbSpeedScheduleKcas = 320.0;
    ac.performance.standardClimbMach = 0.85;
    ac.performance.standardCruiseMach = 0.85;
    ac.performance.standardDescentSpeedScheduleKcas = 340.0;
    ac.performance.standardDescentMach = 0.85;
    ac.performance.transitionAltitudeFt = 10000.0;
    ac.performance.crossOverAltitudeFt = 31000.0;
    ac.performance.costIndex = 35.0;

    return ac;
}

std::string BADAAircraft::getTypeString() const {
    switch (type) {
        case AircraftType::A320: return "Airbus A320";
        case AircraftType::B737_800: return "Boeing 737";
        case AircraftType::B777_300ER: return "Boeing 777";
        case AircraftType::A330_300: return "Airbus A330";
        case AircraftType::B787_9: return "Boeing 787";
        default: return "Custom";
    }
}

double BADAAerodynamics::calculateLiftCoefficient(const BADAAircraft& ac,
                                                   double tasKT,
                                                   double altitudeFt,
                                                   double massKg,
                                                   double bankAngleDeg) {
    if (tasKT <= 0.0) return 0.0;

    auto atm = ISAAtmosphere::atPressureAltitudeFt(altitudeFt);
    double tasMS = tasKT * KT_TO_MS;
    double loadFactor = 1.0 / std::cos(bankAngleDeg * DEG_TO_RAD);
    double liftN = massKg * G * loadFactor;

    double q = 0.5 * atm.densityKgPM3 * tasMS * tasMS;
    if (q <= 0.0) return 0.0;

    return liftN / (q * ac.aerodynamics.wingAreaM2);
}

double BADAAerodynamics::calculateZeroLiftDrag(const BADAAircraft& ac,
                                                double mach,
                                                FlightPhase phase,
                                                double machDragDivergence) {
    double cd0 = ac.aerodynamics.cd0Low;

    if (phase != FlightPhase::Cruise && phase != FlightPhase::Climb && phase != FlightPhase::Descent) {
        if (phase == FlightPhase::Takeoff) cd0 += ac.aerodynamics.flapDragCoeffTakeoff;
        if (phase == FlightPhase::Approach) cd0 += ac.aerodynamics.flapDragCoeffApproach;
        if (phase == FlightPhase::Landing) {
            cd0 += ac.aerodynamics.flapDragCoeffLanding;
            cd0 += ac.aerodynamics.gearDragCoeff;
        }
    }

    double mdd = (machDragDivergence > 0.0) ? machDragDivergence : ac.aerodynamics.machDragDivergence;

    if (mach > mdd - 0.05) {
        double deltaMach = std::max(0.0, mach - (mdd - 0.05));
        double compressDrag = ac.aerodynamics.compressibilityDragCoeff *
                              std::pow(deltaMach, 6.0);
        cd0 += compressDrag;
    }

    return cd0;
}

double BADAAerodynamics::calculateInducedDrag(const BADAAircraft& ac,
                                               double liftCoefficient) {
    double k = 1.0 / (M_PI * ac.aerodynamics.aspectRatio * ac.aerodynamics.oswaldEfficiency);
    return k * liftCoefficient * liftCoefficient;
}

double BADAAerodynamics::calculateDragPolar(const BADAAircraft& ac, double liftCoefficient) {
    return ac.aerodynamics.dragCoeff[0] +
           ac.aerodynamics.dragCoeff[1] * liftCoefficient +
           ac.aerodynamics.dragCoeff[2] * liftCoefficient * liftCoefficient +
           ac.aerodynamics.dragCoeff[3] * std::pow(liftCoefficient, 3) +
           ac.aerodynamics.dragCoeff[4] * std::pow(liftCoefficient, 4) +
           ac.aerodynamics.dragCoeff[5] * std::pow(liftCoefficient, 5);
}

double BADAAerodynamics::calculateTotalDrag(const BADAAircraft& ac,
                                             double tasKT,
                                             double altitudeFt,
                                             double massKg,
                                             FlightPhase phase,
                                             double bankAngleDeg,
                                             double gearDown,
                                             double flapsTakeoff,
                                             double flapsApproach) {
    double cl = calculateLiftCoefficient(ac, tasKT, altitudeFt, massKg, bankAngleDeg);
    double mach = ISAAtmosphere::calculateMachFromTAS(tasKT, ISAAtmosphere::atPressureAltitudeFt(altitudeFt));

    double cd0 = calculateZeroLiftDrag(ac, mach, phase);

    if (gearDown) cd0 += ac.aerodynamics.gearDragCoeff;
    if (flapsTakeoff) cd0 += ac.aerodynamics.flapDragCoeffTakeoff;
    if (flapsApproach) cd0 += ac.aerodynamics.flapDragCoeffApproach;

    double cdi = calculateInducedDrag(ac, cl);

    double cd = cd0 + cdi;

    auto atm = ISAAtmosphere::atPressureAltitudeFt(altitudeFt);
    double tasMS = tasKT * KT_TO_MS;
    double q = 0.5 * atm.densityKgPM3 * tasMS * tasMS;

    return q * ac.aerodynamics.wingAreaM2 * cd;
}

double BADAAerodynamics::calculateMaximumThrust(const BADAAircraft& ac,
                                                 double altitudeFt,
                                                 FlightPhase phase) {
    auto atm = ISAAtmosphere::atPressureAltitudeFt(altitudeFt);
    double sigma = atm.sigma;
    double thrust;

    switch (phase) {
        case FlightPhase::Climb:
            thrust = ac.engine.maxThrustSeaLevelN *
                     (ac.engine.climbThrustCoeff1 * std::pow(sigma, ac.engine.climbThrustCoeff2));
            break;
        case FlightPhase::Cruise:
            thrust = ac.engine.maxThrustSeaLevelN *
                     (ac.engine.cruiseThrustCoeff1 * std::pow(sigma, ac.engine.cruiseThrustCoeff2));
            break;
        case FlightPhase::Descent:
            thrust = ac.engine.approachIdleThrustN;
            break;
        case FlightPhase::Approach:
            thrust = ac.engine.maxThrustSeaLevelN * 0.25;
            break;
        case FlightPhase::Landing:
            thrust = ac.engine.approachIdleThrustN;
            break;
        default:
            thrust = ac.engine.maxThrustSeaLevelN;
            break;
    }

    return std::max(thrust, ac.engine.approachIdleThrustN);
}

double BADAAerodynamics::calculateFuelFlowKgMin(const BADAAircraft& ac,
                                                 double thrustN,
                                                 double altitudeFt,
                                                 double mach,
                                                 FlightPhase phase) {
    if (thrustN <= 0.0) return ac.engine.fuelFlowIdle / 60.0;

    auto atm = ISAAtmosphere::atPressureAltitudeFt(altitudeFt);
    double theta = atm.theta;
    double delta = atm.delta;

    double idleFlow = ac.engine.fuelFlowIdle;

    double thrustPerEngine = thrustN / ac.numEngines;
    double correctedThrust = thrustPerEngine / delta;
    double correctedMach = mach / std::sqrt(theta);

    double fuelFlowPerEngineKgHr =
        ac.engine.fuelFlowCoeff1 * std::pow(correctedThrust, ac.engine.fuelFlowCoeff2) *
        (1.0 + ac.engine.fuelFlowCoeff3 * correctedMach * 1000.0);

    double totalFuelFlowKgHr = ac.numEngines * (fuelFlowPerEngineKgHr + idleFlow / 60.0 * 3600.0);

    if (phase == FlightPhase::Descent) {
        totalFuelFlowKgHr = ac.numEngines * idleFlow;
    }

    return totalFuelFlowKgHr / 60.0;
}

double BADAAerodynamics::calculateRangeKgPerNm(const BADAAircraft& ac,
                                                double tasKT,
                                                double altitudeFt,
                                                double massKg,
                                                FlightPhase phase) {
    double dragN = calculateTotalDrag(ac, tasKT, altitudeFt, massKg, phase);
    double fuelFlowKgMin = calculateFuelFlowKgMin(ac, dragN, altitudeFt,
        ISAAtmosphere::calculateMachFromTAS(tasKT, ISAAtmosphere::atPressureAltitudeFt(altitudeFt)),
        phase);

    if (fuelFlowKgMin <= 0.0) return 0.0;

    double fuelFlowKgPerNm = fuelFlowKgMin * 60.0 / tasKT;
    return 1.0 / fuelFlowKgPerNm;
}

double BADAAerodynamics::calculateStallSpeedKcas(const BADAAircraft& ac,
                                                  double massKg,
                                                  FlightPhase phase) {
    double clMax = ac.aerodynamics.clMaxClean;
    double refWeightKg = ac.aerodynamics.referenceMassKg;

    switch (phase) {
        case FlightPhase::Takeoff:  clMax = ac.aerodynamics.clMaxTakeoff; break;
        case FlightPhase::Approach: clMax = ac.aerodynamics.clMaxApproach; break;
        case FlightPhase::Landing:  clMax = ac.aerodynamics.clMaxLanding; break;
        default: break;
    }

    double baseSpeed = ac.aerodynamics.stallSpeedCleanKcas;
    double speedRatio = std::sqrt(massKg / refWeightKg);
    double clRatio = std::sqrt(ac.aerodynamics.clMaxClean / clMax);

    return baseSpeed * speedRatio * clRatio;
}

}
}
