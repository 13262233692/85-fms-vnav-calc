#pragma once

#include "arinc424_types.h"
#include <vector>
#include <string>
#include <optional>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace fmc {
namespace nav {

using fmc::arinc424::Coordinates;

constexpr double EARTH_MEAN_RADIUS_NM = 3440.065;
constexpr double EARTH_MEAN_RADIUS_KM = 6371.0088;
constexpr double EARTH_MEAN_RADIUS_M = 6371008.8;
constexpr double PI = 3.14159265358979323846;
constexpr double DEG_TO_RAD = PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / PI;
constexpr double NM_TO_KM = 1.852;
constexpr double KM_TO_NM = 1.0 / 1.852;
constexpr double FEET_TO_METERS = 0.3048;
constexpr double METERS_TO_FEET = 3.28084;

struct GreatCircleResult {
    double distanceNm;
    double distanceKm;
    double trueBearing;
    double reverseBearing;

    GreatCircleResult()
        : distanceNm(0.0), distanceKm(0.0), trueBearing(0.0), reverseBearing(0.0) {}
};

GreatCircleResult calculateGreatCircle(const Coordinates& from, const Coordinates& to);

double calculateDistanceNm(const Coordinates& from, const Coordinates& to);
double calculateDistanceKm(const Coordinates& from, const Coordinates& to);
double calculateTrueBearing(const Coordinates& from, const Coordinates& to);
double calculateReverseBearing(double bearing);

struct FlightPlanLeg {
    std::string waypointIdentifier;
    Coordinates position;
    std::optional<double> altitude;
    double distanceFromOrigin;
    double cumulativeDistance;
    double trueTrack;
    double groundTrack;
    std::optional<double> estimatedTimeEnroute;
    std::optional<double> fuelRequired;
    std::string airway;
    std::string remarks;
    size_t sequenceNumber;
};

struct FlightPlan {
    std::string origin;
    std::string destination;
    std::string alternate;
    std::string flightNumber;
    std::string aircraftType;
    std::vector<FlightPlanLeg> legs;
    double totalDistanceNm;
    double totalDistanceKm;
    double estimatedTotalTime;

    FlightPlan()
        : totalDistanceNm(0.0), totalDistanceKm(0.0), estimatedTotalTime(0.0) {}

    void addLeg(const FlightPlanLeg& leg);
    void removeLeg(size_t index);
    void clear();
    size_t legCount() const { return legs.size(); }
    std::optional<FlightPlanLeg> getLeg(size_t index) const;
    void recalculateDistances();
};

enum class CDUInputState {
    Idle,
    EnteringOrigin,
    EnteringDestination,
    EnteringAlternate,
    EnteringRoute,
    EnteringAltitude,
    EnteringAirway,
    EnteringSpeed,
    ReviewingPlan,
    ExecutingPlan
};

enum class CDUKey {
    Key_0, Key_1, Key_2, Key_3, Key_4,
    Key_5, Key_6, Key_7, Key_8, Key_9,
    Key_Dot, Key_Plus, Key_Minus,
    Key_Clear, Key_Delete, Key_Insert,
    Key_Exec, Key_Cancel,
    Key_Left, Key_Right, Key_Up, Key_Down,
    Key_PageUp, Key_PageDown,
    Key_F1, Key_F2, Key_F3, Key_F4, Key_F5,
    Key_Origin, Key_Dest, Key_Alt,
    Key_Route, Key_Legs, Key_Prog,
    Key_Nav, Key_Perf, Key_Init,
    Key_A, Key_B, Key_C, Key_D, Key_E, Key_F,
    Key_G, Key_H, Key_I, Key_J, Key_K, Key_L,
    Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R,
    Key_S, Key_T, Key_U, Key_V, Key_W, Key_X,
    Key_Y, Key_Z, Key_Slash
};

class CDUInputStateMachine {
public:
    CDUInputStateMachine();

    void pressKey(CDUKey key);
    void setInputBuffer(const std::string& buffer);
    const std::string& getInputBuffer() const { return m_inputBuffer; }
    CDUInputState getCurrentState() const { return m_currentState; }
    void setState(CDUInputState state);
    void clearBuffer();

    std::string getStateName() const;
    std::string keyToString(CDUKey key) const;

private:
    CDUInputState m_currentState;
    std::string m_inputBuffer;
    size_t m_cursorPosition;

    void handleCharacterKey(char c);
    void handleNumericKey(int digit);
    void handleClearKey();
    void handleDeleteKey();
    void handleExecKey();
    void handleNavigationKey(CDUKey key);
    void handleFunctionKey(CDUKey key);
};

struct RouteEntry {
    std::string identifier;
    std::string airway;
    std::optional<double> altitude;
    std::optional<double> speed;
    bool isOrigin;
    bool isDestination;
    bool isAlternate;

    RouteEntry()
        : isOrigin(false), isDestination(false), isAlternate(false) {}
};

class RoutePlanner {
public:
    RoutePlanner();

    void setDatabase(const arinc424::ARINC424Database& db);

    bool addRouteEntry(const RouteEntry& entry);
    bool insertRouteEntry(size_t position, const RouteEntry& entry);
    bool removeRouteEntry(size_t position);
    void clearRoute();

    size_t entryCount() const { return m_routeEntries.size(); }
    std::optional<RouteEntry> getEntry(size_t index) const;

    FlightPlan generateFlightPlan();
    std::vector<std::string> validateRoute() const;

    void setOrigin(const std::string& origin);
    void setDestination(const std::string& dest);
    void setAlternate(const std::string& alt);

    const std::string& getOrigin() const { return m_origin; }
    const std::string& getDestination() const { return m_destination; }
    const std::string& getAlternate() const { return m_alternate; }

private:
    arinc424::ARINC424Database m_database;
    std::vector<RouteEntry> m_routeEntries;
    std::string m_origin;
    std::string m_destination;
    std::string m_alternate;

    std::optional<arinc424::Coordinates> resolveCoordinates(const std::string& identifier) const;
    bool resolveAirway(const std::string& airwayId, const std::string& from,
                       const std::string& to, std::vector<arinc424::AirwaySegment>& path) const;
};

}
}
