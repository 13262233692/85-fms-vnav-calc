#pragma once

#include "bada_atmosphere.h"
#include <cmath>
#include <vector>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / M_PI;

namespace fmc {
namespace bada {

struct WindVector {
    double directionDegTrue = 0.0;
    double speedKT = 0.0;

    double eastComponent() const {
        return speedKT * std::sin(directionDegTrue * DEG_TO_RAD);
    }
    double northComponent() const {
        return speedKT * std::cos(directionDegTrue * DEG_TO_RAD);
    }

    static WindVector fromComponents(double eastKT, double northKT) {
        WindVector w;
        w.speedKT = std::sqrt(eastKT * eastKT + northKT * northKT);
        w.directionDegTrue = std::atan2(eastKT, northKT) * RAD_TO_DEG;
        if (w.directionDegTrue < 0.0) w.directionDegTrue += 360.0;
        return w;
    }
};

struct WindLayer {
    double altitudeFt = 0.0;
    WindVector wind;
    double temperatureOffsetK = 0.0;
};

class WindModel {
public:
    WindModel();

    void addWindLayer(double altitudeFt, double windDirDeg, double windSpeedKT,
                      double tempOffsetK = 0.0);

    WindVector getWindAtAltitudeFt(double altitudeFt) const;

    double getTemperatureOffsetAtAltitudeFt(double altitudeFt) const;

    void setStandardAtmosphereWind(double windDirDeg = 270.0, double windSpeedKT = 40.0);

    void setJetStreamWind();
    void setNoWind();

    size_t numLayers() const { return m_layers.size(); }

    const std::vector<WindLayer>& layers() const { return m_layers; }

    void clear() { m_layers.clear(); }

private:
    std::vector<WindLayer> m_layers;
    void sortLayers();
};

class GroundSpeedCalculator {
public:
    static double calculateGroundSpeedKT(double trueAirspeedKT,
                                          double trueTrackDeg,
                                          const WindVector& wind,
                                          double* driftAngleDeg = nullptr,
                                          double* headwindComponent = nullptr,
                                          double* crosswindComponent = nullptr);

    static double calculateRequiredTASForGroundSpeedKT(double groundSpeedKT,
                                                       double trueTrackDeg,
                                                       const WindVector& wind,
                                                       double maxTasKT = 600.0);

    static double calculateDriftAngleDeg(double trueAirspeedKT,
                                          double trueTrackDeg,
                                          const WindVector& wind);

    static double calculateHeadwindComponentKT(double trueTrackDeg,
                                                const WindVector& wind);

    static double calculateCrosswindComponentKT(double trueTrackDeg,
                                                 const WindVector& wind);

    static double calculateFuelFactor(double groundSpeedKT,
                                       double trueAirspeedKT,
                                       double fuelFlowKgMin);
};

}
}
