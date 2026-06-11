#include "bada_wind.h"
#include <algorithm>
#include <cmath>

namespace fmc {
namespace bada {

WindModel::WindModel() {
}

void WindModel::sortLayers() {
    std::sort(m_layers.begin(), m_layers.end(),
              [](const WindLayer& a, const WindLayer& b) {
                  return a.altitudeFt < b.altitudeFt;
              });
}

void WindModel::addWindLayer(double altitudeFt, double windDirDeg,
                              double windSpeedKT, double tempOffsetK) {
    WindLayer layer;
    layer.altitudeFt = altitudeFt;
    layer.wind.directionDegTrue = windDirDeg;
    layer.wind.speedKT = windSpeedKT;
    layer.temperatureOffsetK = tempOffsetK;
    m_layers.push_back(layer);
    sortLayers();
}

WindVector WindModel::getWindAtAltitudeFt(double altitudeFt) const {
    if (m_layers.empty()) {
        return WindVector();
    }

    if (m_layers.size() == 1 || altitudeFt <= m_layers.front().altitudeFt) {
        return m_layers.front().wind;
    }

    if (altitudeFt >= m_layers.back().altitudeFt) {
        return m_layers.back().wind;
    }

    for (size_t i = 1; i < m_layers.size(); ++i) {
        if (altitudeFt <= m_layers[i].altitudeFt) {
            double h0 = m_layers[i-1].altitudeFt;
            double h1 = m_layers[i].altitudeFt;
            double t = (altitudeFt - h0) / (h1 - h0);

            double e0 = m_layers[i-1].wind.eastComponent();
            double n0 = m_layers[i-1].wind.northComponent();
            double e1 = m_layers[i].wind.eastComponent();
            double n1 = m_layers[i].wind.northComponent();

            double e = e0 + t * (e1 - e0);
            double n = n0 + t * (n1 - n0);

            return WindVector::fromComponents(e, n);
        }
    }

    return m_layers.back().wind;
}

double WindModel::getTemperatureOffsetAtAltitudeFt(double altitudeFt) const {
    if (m_layers.empty()) return 0.0;
    if (m_layers.size() == 1 || altitudeFt <= m_layers.front().altitudeFt) {
        return m_layers.front().temperatureOffsetK;
    }
    if (altitudeFt >= m_layers.back().altitudeFt) {
        return m_layers.back().temperatureOffsetK;
    }

    for (size_t i = 1; i < m_layers.size(); ++i) {
        if (altitudeFt <= m_layers[i].altitudeFt) {
            double h0 = m_layers[i-1].altitudeFt;
            double h1 = m_layers[i].altitudeFt;
            double t = (altitudeFt - h0) / (h1 - h0);
            return m_layers[i-1].temperatureOffsetK +
                   t * (m_layers[i].temperatureOffsetK - m_layers[i-1].temperatureOffsetK);
        }
    }
    return m_layers.back().temperatureOffsetK;
}

void WindModel::setStandardAtmosphereWind(double windDirDeg, double windSpeedKT) {
    m_layers.clear();
    addWindLayer(0, windDirDeg, windSpeedKT * 0.3, 0.0);
    addWindLayer(5000, windDirDeg, windSpeedKT * 0.5, 0.0);
    addWindLayer(10000, windDirDeg, windSpeedKT * 0.7, 0.0);
    addWindLayer(20000, windDirDeg, windSpeedKT * 0.9, -5.0);
    addWindLayer(30000, windDirDeg, windSpeedKT * 1.0, -10.0);
    addWindLayer(36000, windDirDeg, windSpeedKT * 1.1, -12.0);
    addWindLayer(43000, windDirDeg, windSpeedKT * 0.95, -8.0);
}

void WindModel::setJetStreamWind() {
    m_layers.clear();
    addWindLayer(0, 250.0, 15.0, 0.0);
    addWindLayer(5000, 260.0, 25.0, 0.0);
    addWindLayer(10000, 270.0, 45.0, 0.0);
    addWindLayer(18000, 275.0, 70.0, -5.0);
    addWindLayer(25000, 275.0, 100.0, -10.0);
    addWindLayer(32000, 270.0, 130.0, -15.0);
    addWindLayer(36000, 265.0, 120.0, -12.0);
    addWindLayer(40000, 260.0, 90.0, -8.0);
    addWindLayer(45000, 255.0, 50.0, -5.0);
}

void WindModel::setNoWind() {
    m_layers.clear();
    addWindLayer(0, 0.0, 0.0, 0.0);
    addWindLayer(50000, 0.0, 0.0, 0.0);
}

double GroundSpeedCalculator::calculateHeadwindComponentKT(double trueTrackDeg,
                                                           const WindVector& wind) {
    double windFrom = wind.directionDegTrue;
    double angleDiff = (trueTrackDeg - windFrom) * DEG_TO_RAD;
    return wind.speedKT * std::cos(angleDiff);
}

double GroundSpeedCalculator::calculateCrosswindComponentKT(double trueTrackDeg,
                                                            const WindVector& wind) {
    double windFrom = wind.directionDegTrue;
    double angleDiff = (trueTrackDeg - windFrom) * DEG_TO_RAD;
    return wind.speedKT * std::sin(angleDiff);
}

double GroundSpeedCalculator::calculateDriftAngleDeg(double trueAirspeedKT,
                                                      double trueTrackDeg,
                                                      const WindVector& wind) {
    double crosswind = calculateCrosswindComponentKT(trueTrackDeg, wind);
    if (trueAirspeedKT <= 0.0) return 0.0;
    double sinDrift = -crosswind / trueAirspeedKT;
    sinDrift = std::max(-1.0, std::min(1.0, sinDrift));
    return std::asin(sinDrift) * RAD_TO_DEG;
}

double GroundSpeedCalculator::calculateGroundSpeedKT(double trueAirspeedKT,
                                                     double trueTrackDeg,
                                                     const WindVector& wind,
                                                     double* driftAngleDeg,
                                                     double* headwindComponent,
                                                     double* crosswindComponent) {
    double drift = calculateDriftAngleDeg(trueAirspeedKT, trueTrackDeg, wind);
    double heading = trueTrackDeg + drift;

    double tasE = trueAirspeedKT * std::sin(heading * DEG_TO_RAD);
    double tasN = trueAirspeedKT * std::cos(heading * DEG_TO_RAD);

    double windE = wind.eastComponent();
    double windN = wind.northComponent();

    double gsE = tasE + windE;
    double gsN = tasN + windN;

    if (driftAngleDeg) *driftAngleDeg = drift;
    if (headwindComponent) *headwindComponent = calculateHeadwindComponentKT(trueTrackDeg, wind);
    if (crosswindComponent) *crosswindComponent = calculateCrosswindComponentKT(trueTrackDeg, wind);

    return std::sqrt(gsE * gsE + gsN * gsN);
}

double GroundSpeedCalculator::calculateRequiredTASForGroundSpeedKT(double groundSpeedKT,
                                                                    double trueTrackDeg,
                                                                    const WindVector& wind,
                                                                    double maxTasKT) {
    if (groundSpeedKT <= 0.0) return 0.0;

    double hw = calculateHeadwindComponentKT(trueTrackDeg, wind);
    double cw = calculateCrosswindComponentKT(trueTrackDeg, wind);

    double tasTarget = groundSpeedKT - hw;
    if (tasTarget <= 0.0) return 100.0;

    for (int iter = 0; iter < 20; ++iter) {
        double drift = calculateDriftAngleDeg(tasTarget, trueTrackDeg, wind);
        double heading = trueTrackDeg + drift;

        double tasE = tasTarget * std::sin(heading * DEG_TO_RAD);
        double tasN = tasTarget * std::cos(heading * DEG_TO_RAD);

        double gsE = tasE + wind.eastComponent();
        double gsN = tasN + wind.northComponent();
        double gs = std::sqrt(gsE * gsE + gsN * gsN);

        double error = groundSpeedKT - gs;
        if (std::abs(error) < 0.1) break;

        tasTarget += error * 1.1;
        tasTarget = std::max(100.0, std::min(maxTasKT, tasTarget));
    }

    return tasTarget;
}

double GroundSpeedCalculator::calculateFuelFactor(double groundSpeedKT,
                                                   double trueAirspeedKT,
                                                   double fuelFlowKgMin) {
    if (fuelFlowKgMin <= 0.0 || groundSpeedKT <= 0.0) return 0.0;
    double fuelFlowKgPerNm = fuelFlowKgMin * 60.0 / groundSpeedKT;
    return 1.0 / fuelFlowKgPerNm;
}

}
}
