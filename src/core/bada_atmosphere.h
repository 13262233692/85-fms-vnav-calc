#pragma once

#include <cmath>
#include <optional>
#include <stdexcept>

namespace fmc {
namespace bada {

constexpr double R = 287.05287;
constexpr double GAMMA_AIR = 1.4;
constexpr double G = 9.80665;
constexpr double TROPOSPHERE_ALTITUDE_M = 11000.0;
constexpr double TROPOPAUSE_TEMPERATURE_K = 216.65;
constexpr double MSL_TEMPERATURE_K = 288.15;
constexpr double MSL_PRESSURE_PA = 101325.0;
constexpr double MSL_DENSITY_KGPM3 = 1.225;
constexpr double TROPOSPHERE_LAPSE_KPM = -0.0065;
constexpr double KT_TO_MS = 0.514444444;
constexpr double MS_TO_KT = 1.0 / KT_TO_MS;
constexpr double FT_TO_M = 0.3048;
constexpr double M_TO_FT = 1.0 / FT_TO_M;
constexpr double NM_TO_M = 1852.0;
constexpr double M_TO_NM = 1.0 / NM_TO_M;
constexpr double KG_TO_LB = 2.20462262185;
constexpr double LB_TO_KG = 1.0 / KG_TO_LB;

struct AtmosphericState {
    double altitudeM = 0.0;
    double altitudeFt = 0.0;
    double temperatureK = 0.0;
    double temperatureC = 0.0;
    double pressurePa = 0.0;
    double pressureHPa = 0.0;
    double densityKgPM3 = 0.0;
    double speedOfSoundMS = 0.0;
    double speedOfSoundKT = 0.0;
    double sigma = 1.0;
    double delta = 1.0;
    double theta = 1.0;

    static AtmosphericState zero() {
        AtmosphericState s;
        s.temperatureK = MSL_TEMPERATURE_K;
        s.temperatureC = MSL_TEMPERATURE_K - 273.15;
        s.pressurePa = MSL_PRESSURE_PA;
        s.pressureHPa = MSL_PRESSURE_PA / 100.0;
        s.densityKgPM3 = MSL_DENSITY_KGPM3;
        s.speedOfSoundMS = std::sqrt(GAMMA_AIR * R * MSL_TEMPERATURE_K);
        s.speedOfSoundKT = s.speedOfSoundMS * MS_TO_KT;
        s.sigma = 1.0;
        s.delta = 1.0;
        s.theta = 1.0;
        return s;
    }
};

class ISAAtmosphere {
public:
    static AtmosphericState atPressureAltitudeFt(double pressureAltitudeFt,
                                                 double temperatureOffsetK = 0.0);

    static AtmosphericState atPressureAltitudeM(double pressureAltitudeM,
                                                 double temperatureOffsetK = 0.0);

    static double pressureAltitudeToGeometricFt(double pressureAltitudeFt,
                                                double temperatureOffsetK = 0.0);

    static double geometricToPressureAltitudeFt(double geometricAltFt,
                                                 double temperatureOffsetK = 0.0);

    static double calculateTASFromCAS(double casKT, double pressureAltitudeFt,
                                       double temperatureOffsetK = 0.0);

    static double calculateCASToTAS(double casKT, const AtmosphericState& atm);

    static double calculateMachFromTAS(double tasKT, const AtmosphericState& atm);

    static double calculateTASFromMach(double mach, const AtmosphericState& atm);

    static double calculateCASFromMach(double mach, const AtmosphericState& atm);

    static double densityRatioSigma(double pressureAltitudeFt,
                                     double temperatureOffsetK = 0.0);

    static double temperatureRatioTheta(double pressureAltitudeFt,
                                         double temperatureOffsetK = 0.0);

    static double pressureRatioDelta(double pressureAltitudeFt,
                                      double temperatureOffsetK = 0.0);

private:
    static double tropospherePressureRatio(double altitudeM);
    static double troposphereTemperature(double altitudeM);
    static double stratospherePressureRatio(double altitudeM);
    static double solveForPressureAltitude(double targetPressureRatio);
};

}
}
