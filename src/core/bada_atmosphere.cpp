#include "bada_atmosphere.h"

namespace fmc {
namespace bada {

double ISAAtmosphere::troposphereTemperature(double altitudeM) {
    return MSL_TEMPERATURE_K + TROPOSPHERE_LAPSE_KPM * altitudeM;
}

double ISAAtmosphere::tropospherePressureRatio(double altitudeM) {
    double exponent = -G / (TROPOSPHERE_LAPSE_KPM * R);
    double tempRatio = troposphereTemperature(altitudeM) / MSL_TEMPERATURE_K;
    return std::pow(tempRatio, exponent);
}

double ISAAtmosphere::stratospherePressureRatio(double altitudeM) {
    double tropoPressure = tropospherePressureRatio(TROPOSPHERE_ALTITUDE_M);
    double exponent = -G * (altitudeM - TROPOSPHERE_ALTITUDE_M) / (R * TROPOPAUSE_TEMPERATURE_K);
    return tropoPressure * std::exp(exponent);
}

AtmosphericState ISAAtmosphere::atPressureAltitudeM(double pressureAltitudeM,
                                                     double temperatureOffsetK) {
    AtmosphericState s;
    s.altitudeM = pressureAltitudeM;
    s.altitudeFt = pressureAltitudeM * M_TO_FT;

    if (pressureAltitudeM <= TROPOSPHERE_ALTITUDE_M) {
        s.temperatureK = troposphereTemperature(pressureAltitudeM) + temperatureOffsetK;
        s.delta = tropospherePressureRatio(pressureAltitudeM);
    } else {
        s.temperatureK = TROPOPAUSE_TEMPERATURE_K + temperatureOffsetK;
        s.delta = stratospherePressureRatio(pressureAltitudeM);
    }

    s.temperatureC = s.temperatureK - 273.15;
    s.theta = s.temperatureK / MSL_TEMPERATURE_K;

    s.pressurePa = MSL_PRESSURE_PA * s.delta;
    s.pressureHPa = s.pressurePa / 100.0;

    if (s.temperatureK > 0.0 && s.theta > 0.0) {
        s.sigma = s.delta / s.theta;
        s.densityKgPM3 = MSL_DENSITY_KGPM3 * s.sigma;
    } else {
        s.sigma = 0.0;
        s.densityKgPM3 = 0.0;
    }

    if (s.temperatureK > 0.0) {
        s.speedOfSoundMS = std::sqrt(GAMMA_AIR * R * s.temperatureK);
        s.speedOfSoundKT = s.speedOfSoundMS * MS_TO_KT;
    } else {
        s.speedOfSoundMS = 0.0;
        s.speedOfSoundKT = 0.0;
    }

    return s;
}

AtmosphericState ISAAtmosphere::atPressureAltitudeFt(double pressureAltitudeFt,
                                                     double temperatureOffsetK) {
    return atPressureAltitudeM(pressureAltitudeFt * FT_TO_M, temperatureOffsetK);
}

double ISAAtmosphere::densityRatioSigma(double pressureAltitudeFt,
                                         double temperatureOffsetK) {
    auto atm = atPressureAltitudeFt(pressureAltitudeFt, temperatureOffsetK);
    return atm.sigma;
}

double ISAAtmosphere::temperatureRatioTheta(double pressureAltitudeFt,
                                             double temperatureOffsetK) {
    auto atm = atPressureAltitudeFt(pressureAltitudeFt, temperatureOffsetK);
    return atm.theta;
}

double ISAAtmosphere::pressureRatioDelta(double pressureAltitudeFt,
                                          double temperatureOffsetK) {
    auto atm = atPressureAltitudeFt(pressureAltitudeFt, temperatureOffsetK);
    return atm.delta;
}

double ISAAtmosphere::calculateMachFromTAS(double tasKT, const AtmosphericState& atm) {
    if (atm.speedOfSoundKT <= 0.0) return 0.0;
    return tasKT / atm.speedOfSoundKT;
}

double ISAAtmosphere::calculateTASFromMach(double mach, const AtmosphericState& atm) {
    return mach * atm.speedOfSoundKT;
}

double ISAAtmosphere::calculateCASFromMach(double mach, const AtmosphericState& atm) {
    double tasMS = mach * atm.speedOfSoundMS;
    double qc = 0.5 * atm.densityKgPM3 * tasMS * tasMS;
    if (qc <= 0.0) return 0.0;

    const double p0 = MSL_PRESSURE_PA;
    const double gamma = GAMMA_AIR;
    const double rho0 = MSL_DENSITY_KGPM3;

    if (mach < 1.0) {
        double pOverP0 = (qc / p0 + 1.0);
        double casMS = std::sqrt(2.0 * p0 / rho0 *
            (std::pow(pOverP0, (gamma - 1.0) / gamma) - 1.0));
        return casMS * MS_TO_KT;
    } else {
        double y0 = std::pow((gamma + 1.0) / 2.0, gamma / (gamma - 1.0));
        double pOverP0 = qc / p0 / y0 + 1.0;
        double casMS = std::sqrt(gamma * R * MSL_TEMPERATURE_K *
            std::log(pOverP0));
        return casMS * MS_TO_KT;
    }
}

double ISAAtmosphere::calculateCASToTAS(double casKT, const AtmosphericState& atm) {
    double casMS = casKT * KT_TO_MS;
    const double gamma = GAMMA_AIR;
    const double p0 = MSL_PRESSURE_PA;
    const double rho0 = MSL_DENSITY_KGPM3;

    double qc;
    if (casMS < 340.0) {
        double temp = std::pow(1.0 + (gamma - 1.0) * rho0 * casMS * casMS / (2.0 * gamma * p0),
                               gamma / (gamma - 1.0));
        qc = p0 * (temp - 1.0);
    } else {
        double y0 = std::pow((gamma + 1.0) / 2.0, gamma / (gamma - 1.0));
        double temp = std::exp(casMS * casMS / (gamma * R * MSL_TEMPERATURE_K));
        qc = p0 * y0 * (temp - 1.0);
    }

    double p = atm.pressurePa;
    double rho = atm.densityKgPM3;

    if (rho <= 0.0) return 0.0;

    double tasMS;
    if (qc / p < 0.893) {
        double temp = std::pow(1.0 + qc / p, (gamma - 1.0) / gamma) - 1.0;
        tasMS = std::sqrt(2.0 * gamma * R * atm.temperatureK / (gamma - 1.0) * temp);
    } else {
        double a = gamma * R * atm.temperatureK;
        double b = 2.0 * (gamma - 1.0) / (gamma + 1.0);
        double temp = 1.0 + qc / p * b;
        tasMS = std::sqrt(a * std::log(temp));
    }

    return tasMS * MS_TO_KT;
}

double ISAAtmosphere::calculateTASFromCAS(double casKT, double pressureAltitudeFt,
                                           double temperatureOffsetK) {
    auto atm = atPressureAltitudeFt(pressureAltitudeFt, temperatureOffsetK);
    return calculateCASToTAS(casKT, atm);
}

double ISAAtmosphere::solveForPressureAltitude(double targetPressureRatio) {
    if (targetPressureRatio <= 0.0) return 20000.0;

    double lo = 0.0;
    double hi = 20000.0;
    for (int iter = 0; iter < 50; ++iter) {
        double mid = (lo + hi) * 0.5;
        double ratio = mid <= TROPOSPHERE_ALTITUDE_M
                       ? tropospherePressureRatio(mid)
                       : stratospherePressureRatio(mid);
        if (ratio > targetPressureRatio) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) * 0.5;
}

double ISAAtmosphere::pressureAltitudeToGeometricFt(double pressureAltitudeFt,
                                                    double temperatureOffsetK) {
    return pressureAltitudeFt;
}

double ISAAtmosphere::geometricToPressureAltitudeFt(double geometricAltFt,
                                                    double temperatureOffsetK) {
    return geometricAltFt;
}

}
}
