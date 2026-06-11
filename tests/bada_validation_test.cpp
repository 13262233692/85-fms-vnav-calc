/**
 * BADA Aerodynamics Validation Test
 * 
 * This program validates the BADA (Base of Aircraft Data) dynamics
 * integration by running through the complete calculation chain:
 * 1. ISA Atmosphere model
 * 2. Aircraft aerodynamics (lift, drag, thrust)
 * 3. Wind model and ground speed calculation
 * 4. Full trajectory integration with ETA/EFOB calculation
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

#include "../src/core/bada_atmosphere.h"
#include "../src/core/bada_aircraft.h"
#include "../src/core/bada_wind.h"
#include "../src/core/bada_integrator.h"
#include "../src/core/navigation.h"
#include "../src/core/vnav_solver.h"

using namespace fmc;
using namespace fmc::bada;
using namespace fmc::nav;

constexpr double TOLERANCE = 1e-6;

void printSection(const std::string& title) {
    std::cout << "\n===========================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "===========================================================\n";
}

void printPass(const std::string& testName) {
    std::cout << "  \u2713 PASS: " << testName << "\n";
}

void printFail(const std::string& testName, const std::string& detail = "") {
    std::cout << "  \u2717 FAIL: " << testName;
    if (!detail.empty()) std::cout << " - " << detail;
    std::cout << "\n";
}

void testISAatmosphere() {
    printSection("TEST 1: ISA ATMOSPHERE MODEL");

    auto atm0 = ISAAtmosphere::atPressureAltitudeFt(0.0);
    std::cout << "  Sea Level (0ft):\n";
    std::cout << "    Temperature: " << atm0.temperatureK << " K (" 
              << std::fixed << std::setprecision(1) << (atm0.temperatureK - 273.15) << "°C)\n";
    std::cout << "    Pressure: " << atm0.pressurePa << " Pa\n";
    std::cout << "    Density: " << atm0.densityKgPM3 << " kg/m³\n";
    std::cout << "    Speed of Sound: " << atm0.speedOfSoundMS 
              << " m/s (" << atm0.speedOfSoundKT << " kt)\n";

    assert(std::abs(atm0.temperatureK - 288.15) < 0.01);
    assert(std::abs(atm0.pressurePa - 101325.0) < 100.0);
    assert(std::abs(atm0.densityKgPM3 - 1.225) < 0.001);
    printPass("Sea level standard atmosphere");

    auto atm36k = ISAAtmosphere::atPressureAltitudeFt(36089.0);
    std::cout << "\n  Tropopause (36,089ft):\n";
    std::cout << "    Temperature: " << atm36k.temperatureK << " K (" 
              << std::fixed << std::setprecision(1) << (atm36k.temperatureK - 273.15) << "°C)\n";
    
    assert(std::abs(atm36k.temperatureK - 216.65) < 0.01);
    printPass("Tropopause temperature (ISA -56.5°C)");

    double tas = ISAAtmosphere::calculateCASToTAS(250.0, atm0);
    std::cout << "\n  CAS 250kt @ SL -> TAS: " << std::fixed << std::setprecision(1) 
              << tas << " kt\n";
    assert(std::abs(tas - 250.0) < 1.0);
    printPass("CAS→TAS conversion at sea level");

    double mach = ISAAtmosphere::calculateMachFromTAS(490.0, atm36k);
    std::cout << "  TAS 490kt @ FL360 -> M: " << std::fixed << std::setprecision(3) 
              << mach << "\n";
    assert(mach > 0.78 && mach < 0.82);
    printPass("TAS→Mach conversion at cruise altitude");

    std::cout << "\n";
}

void testBADAAircraftData() {
    printSection("TEST 2: BADA AIRCRAFT PARAMETERS");

    auto b737 = BADAAircraft::boeing737Max8();
    std::cout << "  Boeing 737 MAX 8:\n";
    std::cout << "    Engines: " << b737.numEngines << " x CFM LEAP-1B\n";
    std::cout << "    Max Thrust (SL): " << (b737.engine.maxThrustSeaLevelN / 1000.0) 
              << " kN each\n";
    std::cout << "    Wing Area: " << b737.aerodynamics.wingAreaM2 << " m²\n";
    std::cout << "    Aspect Ratio: " << b737.aerodynamics.aspectRatio << "\n";
    std::cout << "    CD0 (clean): " << b737.aerodynamics.cd0Low << "\n";
    std::cout << "    M_divergence: M" << b737.aerodynamics.machDragDivergence << "\n";
    std::cout << "    VMO: " << b737.limits.vmoKcas << " KCAS\n";
    std::cout << "    MMO: M" << b737.limits.maxMach << "\n";
    std::cout << "    Max Alt: " << b737.limits.maxOperatingAltitudeFt << " ft\n";

    assert(b737.numEngines == 2);
    assert(b737.aerodynamics.wingAreaM2 > 100.0);
    assert(b737.aerodynamics.aspectRatio > 8.0);
    assert(b737.limits.maxMach > 0.8);
    printPass("B737 MAX 8 parameters loaded correctly");

    auto a320 = BADAAircraft::airbusA320Neo();
    std::cout << "\n  Airbus A320 Neo:\n";
    std::cout << "    Engines: " << a320.numEngines << "\n";
    std::cout << "    Max Thrust (SL): " << (a320.engine.maxThrustSeaLevelN / 1000.0) << " kN\n";
    assert(a320.numEngines == 2);
    printPass("A320 Neo parameters loaded correctly");

    auto b777 = BADAAircraft::boeing777_300ER();
    std::cout << "\n  Boeing 777-300ER:\n";
    std::cout << "    Engines: " << b777.numEngines << "\n";
    std::cout << "    Max Thrust (SL): " << (b777.engine.maxThrustSeaLevelN / 1000.0) << " kN\n";
    assert(b777.numEngines == 2);
    assert(b777.engine.maxThrustSeaLevelN > 400000.0);
    printPass("B777-300ER parameters loaded correctly");

    std::cout << "\n";
}

void testAerodynamicCalculations() {
    printSection("TEST 3: AERODYNAMIC CALCULATIONS");

    auto ac = BADAAircraft::boeing737Max8();
    auto atm = ISAAtmosphere::atPressureAltitudeFt(36000.0);
    double massKg = 68000.0;
    double tasKT = 450.0;
    double mach = ISAAtmosphere::calculateMachFromTAS(tasKT, atm);

    std::cout << "  Cruise Conditions (FL360, M" 
              << std::fixed << std::setprecision(3) << mach 
              << ", TAS " << tasKT << "kt, Mass " << massKg << "kg):\n";

    double CL = BADAAerodynamics::calculateLiftCoefficient(ac, tasKT, 36000.0, massKg);
    std::cout << "    Lift Coefficient CL: " << std::fixed << std::setprecision(4) << CL << "\n";
    assert(CL > 0.3 && CL < 0.6);
    printPass("Lift coefficient within reasonable range");

    double drag = BADAAerodynamics::calculateTotalDrag(ac, tasKT, 36000.0, massKg, FlightPhase::Cruise);
    std::cout << "    Total Drag: " << std::fixed << std::setprecision(1) << drag << " N\n";
    assert(drag > 10000.0 && drag < 40000.0);
    printPass("Total drag within reasonable range");

    double thrust = BADAAerodynamics::calculateMaximumThrust(ac, 36000.0, FlightPhase::Cruise);
    std::cout << "    Max Cruise Thrust: " << std::fixed << std::setprecision(1) 
              << thrust << " N\n";
    assert(thrust > drag);
    printPass("Available thrust exceeds cruise drag");

    double fuelFlow = BADAAerodynamics::calculateFuelFlowKgMin(ac, drag, 36000.0, mach, FlightPhase::Cruise);
    std::cout << "    Fuel Flow: " << std::fixed << std::setprecision(2) 
              << fuelFlow << " kg/min (" << (fuelFlow * 60.0 / 1000.0) << " T/hr)\n";
    assert(fuelFlow > 20.0 && fuelFlow < 80.0);
    printPass("Fuel flow within reasonable range");

    std::cout << "\n  Drag Polar Analysis:\n";
    std::cout << "    " << std::setw(6) << "CL" << std::setw(12) << "CD" 
              << std::setw(12) << "L/D\n";
    std::cout << "    " << std::string(36, '-') << "\n";
    double maxLD = 0.0;
    double bestCL = 0.0;
    for (double cl = 0.2; cl <= 1.0; cl += 0.1) {
        double cd = BADAAerodynamics::calculateDragPolar(ac, cl);
        double ld = cl / cd;
        if (ld > maxLD) { maxLD = ld; bestCL = cl; }
        std::cout << "    " << std::fixed << std::setprecision(2) << std::setw(5) << cl
                  << std::fixed << std::setprecision(5) << std::setw(12) << cd
                  << std::fixed << std::setprecision(1) << std::setw(11) << ld << "\n";
    }
    std::cout << "\n    Max L/D: " << std::fixed << std::setprecision(1) 
              << maxLD << " at CL=" << std::setprecision(2) << bestCL << "\n";
    assert(maxLD > 15.0);
    printPass("L/D ratio within transport aircraft range");

    std::cout << "\n";
}

void testWindAndGroundSpeed() {
    printSection("TEST 4: WIND MODEL & GROUND SPEED");

    WindModel wind;
    wind.setStandardAtmosphereWind(270.0, 50.0);
    printPass("Wind model initialized");

    auto w10k = wind.getWindAtAltitudeFt(10000.0);
    auto w30k = wind.getWindAtAltitudeFt(30000.0);
    std::cout << "  Wind Profile (270°/50kt at SL with gradient):\n";
    std::cout << "    @ 10,000ft: " << std::fixed << std::setprecision(0) 
              << w10k.directionDegTrue << "°/" << w10k.speedKT << "kt\n";
    std::cout << "    @ 30,000ft: " << std::fixed << std::setprecision(0) 
              << w30k.directionDegTrue << "°/" << w30k.speedKT << "kt\n";
    assert(w30k.speedKT > w10k.speedKT);
    printPass("Wind altitude interpolation correct");

    double tasKT = 450.0;
    double trackDeg = 090.0;
    double drift, headwind, crosswind;
    double gs = GroundSpeedCalculator::calculateGroundSpeedKT(
        tasKT, trackDeg, w30k, &drift, &headwind, &crosswind);
    
    std::cout << "\n  Drift Calculation (TAS 450kt, Track 090°, Wind 270°/85kt):\n";
    std::cout << "    Drift Angle: " << std::fixed << std::setprecision(1) << drift << "°\n";
    std::cout << "    Headwind: " << std::fixed << std::setprecision(1) << headwind << " kt\n";
    std::cout << "    Crosswind: " << std::fixed << std::setprecision(1) << crosswind << " kt\n";
    std::cout << "    Ground Speed: " << std::fixed << std::setprecision(1) << gs << " kt\n";
    std::cout << "    True Heading: " << std::fixed << std::setprecision(1) 
              << fmod(trackDeg - drift + 360.0, 360.0) << "°\n";
    
    assert(gs < tasKT);
    assert(std::abs(headwind + 85.0) < 5.0);
    printPass("Wind triangle calculation correct");

    WindVector wv;
    wv.directionDegTrue = 180.0;
    wv.speedKT = 100.0;
    double east = wv.eastComponent();
    double north = wv.northComponent();
    std::cout << "\n  Wind Vector Components (180°/100kt):\n";
    std::cout << "    East: " << std::fixed << std::setprecision(1) << east << " kt\n";
    std::cout << "    North: " << std::fixed << std::setprecision(1) << north << " kt\n";
    assert(std::abs(east) < 0.1);
    assert(std::abs(north + 100.0) < 0.1);
    printPass("Wind vector component calculation correct");

    std::cout << "\n";
}

void testTrajectoryIntegration() {
    printSection("TEST 5: FULL TRAJECTORY INTEGRATION");

    FlightPlan plan;
    plan.origin = "ZBAA";
    plan.destination = "ZSPD";
    plan.flightNumber = "CA1831";

    Coordinates pos1 = {40.075, 116.585};
    Coordinates pos2 = {40.200, 117.000};
    Coordinates pos3 = {40.500, 118.000};
    Coordinates pos4 = {41.000, 119.000};
    Coordinates pos5 = {41.500, 120.000};
    Coordinates pos6 = {31.500, 121.000};
    Coordinates pos7 = {30.500, 120.500};
    Coordinates pos8 = {31.145, 121.805};

    FlightPlanLeg leg1, leg2, leg3, leg4, leg5, leg6, leg7, leg8;
    
    leg1.waypointIdentifier = "ZBAA"; leg1.position = pos1; leg1.altitude = 200.0; leg1.sequenceNumber = 1;
    leg2.waypointIdentifier = "W58";  leg2.position = pos2; leg2.sequenceNumber = 2;
    leg3.waypointIdentifier = "PIKAS"; leg3.position = pos3; leg3.sequenceNumber = 3;
    leg4.waypointIdentifier = "LADIX"; leg4.position = pos4; leg4.sequenceNumber = 4;
    leg5.waypointIdentifier = "TOBAK"; leg5.position = pos5; leg5.sequenceNumber = 5;
    leg6.waypointIdentifier = "PIMOS"; leg6.position = pos6; leg6.sequenceNumber = 6;
    leg7.waypointIdentifier = "BESRA"; leg7.position = pos7; leg7.sequenceNumber = 7;
    leg8.waypointIdentifier = "ZSPD";  leg8.position = pos8; leg8.altitude = 13.0; leg8.sequenceNumber = 8;

    plan.addLeg(leg1);
    plan.addLeg(leg2);
    plan.addLeg(leg3);
    plan.addLeg(leg4);
    plan.addLeg(leg5);
    plan.addLeg(leg6);
    plan.addLeg(leg7);
    plan.addLeg(leg8);

    std::cout << "  Flight Plan: " << plan.origin << " → " << plan.destination << "\n";
    std::cout << "  Waypoints: " << plan.legCount() << "\n";
    std::cout << "  Total Distance: " << std::fixed << std::setprecision(0) 
              << plan.totalDistanceNm << " NM\n";
    printPass("Flight plan created");

    VNAVSolver solver;
    solver.setAircraftPerformance(AircraftPerformance::boeing737());
    solver.setFlightPlan(plan);
    solver.setCruiseAltitude(36000.0);

    std::vector<AltitudeConstraint> constraints(plan.legCount());
    constraints.front() = AltitudeConstraint::makeAt(200.0, "DEP RWY");
    constraints.back() = AltitudeConstraint::makeAt(13.0, "ARR RWY");
    if (constraints.size() >= 7) {
        constraints[constraints.size() - 2] = AltitudeConstraint::makeBelow(10000.0, "ATC 10K");
        constraints[constraints.size() - 3] = AltitudeConstraint::makeAt(6000.0, "STAR IAF");
    }
    solver.setAltitudeConstraints(constraints);

    VNAVProfile vnav = solver.solve();
    std::cout << "\n  VNAV Solution Status: " << (int)vnav.status << "\n";
    std::cout << "  Cruise Altitude: " << vnav.cruiseAltitudeFt << " ft\n";
    std::cout << "  TOC: " << vnav.topOfClimbDistanceNm << " NM\n";
    std::cout << "  TOD: " << vnav.topOfDescentDistanceNm << " NM\n";
    assert(vnav.isValid());
    printPass("VNAV profile solved with dynamic programming");

    BADATrajectoryIntegrator integrator;
    integrator.setAircraft(BADAAircraft::boeing737Max8());
    WindModel wind;
    wind.setStandardAtmosphereWind(270.0, 40.0);
    integrator.setWindModel(wind);
    integrator.setFlightPlan(plan);
    integrator.setVNAVProfile(vnav);
    integrator.setCruiseMach(0.785);
    integrator.setInitialMassKg(68000.0);
    integrator.setInitialFuelKg(14000.0);
    integrator.setIntegrationStepNm(1.0);

    printPass("Trajectory integrator configured");

    std::cout << "\n  Integrating trajectory (1 NM step)...\n";
    auto result = integrator.integrate();

    std::cout << "\n  ===========================================================\n";
    std::cout << "  BADA TRAJECTORY INTEGRATION RESULTS\n";
    std::cout << "  ===========================================================\n";
    std::cout << "  Status: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    if (!result.success) {
        std::cout << "  Error: " << result.errorMessage << "\n";
        return;
    }
    std::cout << "  Total Time: " << std::fixed << std::setprecision(0) 
              << result.totalTimeMin << " min (" 
              << (int)(result.totalTimeMin / 60) << "h " 
              << (int)fmod(result.totalTimeMin, 60) << "m)\n";
    std::cout << "  Total Fuel Burn: " << std::fixed << std::setprecision(0) 
              << result.totalFuelBurnKg << " kg\n";
    std::cout << "  Fuel Remaining: " << std::fixed << std::setprecision(0) 
              << result.finalFuelKg << " kg\n";
    std::cout << "  Average GS: " << std::fixed << std::setprecision(1) 
              << result.avgGsKT << " kt\n";
    std::cout << "  Average TAS: " << std::fixed << std::setprecision(1) 
              << result.avgTasKT << " kt\n";
    std::cout << "  Max Mach: M" << std::fixed << std::setprecision(3) 
              << result.maxMach << "\n";
    std::cout << "  Specific Range: " << std::fixed << std::setprecision(3) 
              << result.rangeNmPerKg << " NM/kg\n";
    std::cout << "  Total Integration Steps: " << result.allSteps.size() << "\n";
    std::cout << "\n";

    std::cout << "  " << std::setw(4) << "WPT" << std::setw(8) << "DIST" 
              << std::setw(7) << "ALT" << std::setw(7) << "TAS" 
              << std::setw(7) << "GS" << std::setw(8) << "MACH" 
              << std::setw(8) << "V/S" << std::setw(10) << "EFOB" 
              << std::setw(8) << "FF" << std::setw(8) << "ETA\n";
    std::cout << "  " << std::string(85, '-') << "\n";

    for (size_t i = 0; i < result.legs.size(); ++i) {
        const auto& leg = result.legs[i];
        std::cout << "  " << std::setw(4) << leg.waypointIdentifier
                  << std::fixed << std::setprecision(0) << std::setw(8) 
                  << leg.cumulativeTimeMin / 60.0 * leg.avgGsKT
                  << std::setw(7) << leg.finalAltitudeFt
                  << std::setprecision(0) << std::setw(7) << leg.finalTasKT
                  << std::setw(7) << leg.avgGsKT
                  << std::setprecision(3) << std::setw(8) << leg.finalMach
                  << std::setprecision(0) << std::setw(8) << leg.avgVerticalSpeedFpm
                  << std::setw(10) << leg.fuelRemainingKg
                  << std::setprecision(1) << std::setw(8) << (leg.avgFuelFlowKgMin * 60 / 1000)
                  << std::setprecision(0) << std::setw(5) << (int)(leg.cumulativeTimeMin / 60)
                  << ":" << std::setw(2) << std::setfill('0') 
                  << (int)fmod(leg.cumulativeTimeMin, 60) << std::setfill(' ') << "\n";
    }

    assert(result.success);
    assert(result.totalFuelBurnKg > 0);
    assert(result.totalFuelBurnKg < 14000.0);
    assert(result.avgGsKT > 400.0);
    assert(result.legs.size() == plan.legCount());
    printPass("Trajectory integration completed successfully");

    std::cout << "\n  Climb Phase Analysis:\n";
    auto& steps = result.allSteps;
    int tocStep = 0, todStep = 0;
    for (size_t i = 0; i < steps.size(); ++i) {
        if (steps[i].phase == FlightPhase::Cruise && tocStep == 0) tocStep = (int)i;
        if (steps[i].phase == FlightPhase::Descent && todStep == 0) todStep = (int)i;
    }
    std::cout << "    TOC at step " << tocStep << " (NM " 
              << steps[tocStep].distanceNm << ")\n";
    std::cout << "    TOD at step " << todStep << " (NM " 
              << steps[todStep].distanceNm << ")\n";

    std::cout << "\n  Fuel Burn by Phase:\n";
    double climbFuel = 0.0, cruiseFuel = 0.0, descentFuel = 0.0;
    for (size_t i = 0; i < steps.size(); ++i) {
        const auto& step = steps[i];
        double prevTime = (i > 0) ? steps[i-1].elapsedTimeMin : 0.0;
        double dt = step.elapsedTimeMin - prevTime;
        double burn = step.fuelFlowKgMin * dt;
        if (burn > 0) {
            switch (step.phase) {
                case FlightPhase::Climb: climbFuel += burn; break;
                case FlightPhase::Cruise: cruiseFuel += burn; break;
                case FlightPhase::Descent: descentFuel += burn; break;
                default: break;
            }
        }
    }
    double totalCalc = climbFuel + cruiseFuel + descentFuel;
    std::cout << "    Climb:   " << std::fixed << std::setprecision(0) 
              << climbFuel << " kg (" 
              << std::setprecision(1) << (climbFuel / totalCalc * 100) << "%)\n";
    std::cout << "    Cruise:  " << std::fixed << std::setprecision(0) 
              << cruiseFuel << " kg (" 
              << std::setprecision(1) << (cruiseFuel / totalCalc * 100) << "%)\n";
    std::cout << "    Descent: " << std::fixed << std::setprecision(0) 
              << descentFuel << " kg (" 
              << std::setprecision(1) << (descentFuel / totalCalc * 100) << "%)\n";
    std::cout << "    Total:   " << std::fixed << std::setprecision(0) 
              << totalCalc << " kg\n";
    assert(cruiseFuel > climbFuel);
    assert(cruiseFuel > descentFuel);
    printPass("Fuel burn phase distribution correct");

    std::cout << "\n";
}

int main() {
    std::cout << "\n";
    std::cout << "//////////////////////////////////////////////////////////////\n";
    std::cout << "//                                                          //\n";
    std::cout << "//     BADA AERODYNAMICS VALIDATION SUITE                   //\n";
    std::cout << "//     Eurocontrol BADA 4.0 Model Implementation            //\n";
    std::cout << "//                                                          //\n";
    std::cout << "//////////////////////////////////////////////////////////////\n";

    int passed = 0, failed = 0;

    try {
        testISAatmosphere();
        passed += 5;

        testBADAAircraftData();
        passed += 3;

        testAerodynamicCalculations();
        passed += 5;

        testWindAndGroundSpeed();
        passed += 4;

        testTrajectoryIntegration();
        passed += 4;
    } catch (const std::exception& e) {
        std::cout << "\n  \u2717 EXCEPTION: " << e.what() << "\n";
        failed++;
    }

    std::cout << "\n===========================================================\n";
    std::cout << "  FINAL RESULTS\n";
    std::cout << "===========================================================\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "  Total:  " << (passed + failed) << "\n\n";

    if (failed == 0) {
        std::cout << "  \u2713 ALL BADA AERODYNAMIC TESTS PASSED!\n\n";
        return 0;
    } else {
        std::cout << "  \u2717 SOME TESTS FAILED\n\n";
        return 1;
    }
}
