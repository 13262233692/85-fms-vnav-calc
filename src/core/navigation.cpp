#include "navigation.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace fmc {
namespace nav {

GreatCircleResult calculateGreatCircle(const Coordinates& from, const Coordinates& to) {
    GreatCircleResult result;

    double lat1 = from.latitude * DEG_TO_RAD;
    double lon1 = from.longitude * DEG_TO_RAD;
    double lat2 = to.latitude * DEG_TO_RAD;
    double lon2 = to.longitude * DEG_TO_RAD;

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    result.distanceNm = EARTH_MEAN_RADIUS_NM * c;
    result.distanceKm = EARTH_MEAN_RADIUS_KM * c;

    double y = std::sin(dLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) -
               std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
    double bearing = std::atan2(y, x) * RAD_TO_DEG;
    result.trueBearing = std::fmod(bearing + 360.0, 360.0);

    double revLon1 = to.longitude * DEG_TO_RAD;
    double revLat1 = to.latitude * DEG_TO_RAD;
    double revLon2 = from.longitude * DEG_TO_RAD;
    double revLat2 = from.latitude * DEG_TO_RAD;
    double revDLon = revLon2 - revLon1;

    double revY = std::sin(revDLon) * std::cos(revLat2);
    double revX = std::cos(revLat1) * std::sin(revLat2) -
                  std::sin(revLat1) * std::cos(revLat2) * std::cos(revDLon);
    double revBearing = std::atan2(revY, revX) * RAD_TO_DEG;
    result.reverseBearing = std::fmod(revBearing + 360.0, 360.0);

    return result;
}

double calculateDistanceNm(const Coordinates& from, const Coordinates& to) {
    return calculateGreatCircle(from, to).distanceNm;
}

double calculateDistanceKm(const Coordinates& from, const Coordinates& to) {
    return calculateGreatCircle(from, to).distanceKm;
}

double calculateTrueBearing(const Coordinates& from, const Coordinates& to) {
    return calculateGreatCircle(from, to).trueBearing;
}

double calculateReverseBearing(double bearing) {
    return std::fmod(bearing + 180.0, 360.0);
}

void FlightPlan::addLeg(const FlightPlanLeg& leg) {
    legs.push_back(leg);
    legs.back().sequenceNumber = legs.size();
    recalculateDistances();
}

void FlightPlan::removeLeg(size_t index) {
    if (index < legs.size()) {
        legs.erase(legs.begin() + index);
        for (size_t i = index; i < legs.size(); ++i) {
            legs[i].sequenceNumber = i + 1;
        }
        recalculateDistances();
    }
}

void FlightPlan::clear() {
    legs.clear();
    totalDistanceNm = 0.0;
    totalDistanceKm = 0.0;
    estimatedTotalTime = 0.0;
}

std::optional<FlightPlanLeg> FlightPlan::getLeg(size_t index) const {
    if (index < legs.size()) {
        return legs[index];
    }
    return std::nullopt;
}

void FlightPlan::recalculateDistances() {
    totalDistanceNm = 0.0;
    totalDistanceKm = 0.0;

    if (legs.size() < 2) {
        for (auto& leg : legs) {
            leg.distanceFromOrigin = 0.0;
            leg.cumulativeDistance = 0.0;
        }
        return;
    }

    legs[0].distanceFromOrigin = 0.0;
    legs[0].cumulativeDistance = 0.0;

    for (size_t i = 1; i < legs.size(); ++i) {
        GreatCircleResult gc = calculateGreatCircle(legs[i - 1].position, legs[i].position);
        legs[i].distanceFromOrigin = gc.distanceNm;
        legs[i].trueTrack = gc.trueBearing;
        legs[i].groundTrack = gc.trueBearing;
        totalDistanceNm += gc.distanceNm;
        totalDistanceKm += gc.distanceKm;
        legs[i].cumulativeDistance = totalDistanceNm;
    }

    estimatedTotalTime = totalDistanceNm / 450.0 * 60.0;
}

CDUInputStateMachine::CDUInputStateMachine()
    : m_currentState(CDUInputState::Idle), m_cursorPosition(0) {}

void CDUInputStateMachine::pressKey(CDUKey key) {
    switch (key) {
        case CDUKey::Key_0: handleNumericKey(0); break;
        case CDUKey::Key_1: handleNumericKey(1); break;
        case CDUKey::Key_2: handleNumericKey(2); break;
        case CDUKey::Key_3: handleNumericKey(3); break;
        case CDUKey::Key_4: handleNumericKey(4); break;
        case CDUKey::Key_5: handleNumericKey(5); break;
        case CDUKey::Key_6: handleNumericKey(6); break;
        case CDUKey::Key_7: handleNumericKey(7); break;
        case CDUKey::Key_8: handleNumericKey(8); break;
        case CDUKey::Key_9: handleNumericKey(9); break;

        case CDUKey::Key_A: handleCharacterKey('A'); break;
        case CDUKey::Key_B: handleCharacterKey('B'); break;
        case CDUKey::Key_C: handleCharacterKey('C'); break;
        case CDUKey::Key_D: handleCharacterKey('D'); break;
        case CDUKey::Key_E: handleCharacterKey('E'); break;
        case CDUKey::Key_F: handleCharacterKey('F'); break;
        case CDUKey::Key_G: handleCharacterKey('G'); break;
        case CDUKey::Key_H: handleCharacterKey('H'); break;
        case CDUKey::Key_I: handleCharacterKey('I'); break;
        case CDUKey::Key_J: handleCharacterKey('J'); break;
        case CDUKey::Key_K: handleCharacterKey('K'); break;
        case CDUKey::Key_L: handleCharacterKey('L'); break;
        case CDUKey::Key_M: handleCharacterKey('M'); break;
        case CDUKey::Key_N: handleCharacterKey('N'); break;
        case CDUKey::Key_O: handleCharacterKey('O'); break;
        case CDUKey::Key_P: handleCharacterKey('P'); break;
        case CDUKey::Key_Q: handleCharacterKey('Q'); break;
        case CDUKey::Key_R: handleCharacterKey('R'); break;
        case CDUKey::Key_S: handleCharacterKey('S'); break;
        case CDUKey::Key_T: handleCharacterKey('T'); break;
        case CDUKey::Key_U: handleCharacterKey('U'); break;
        case CDUKey::Key_V: handleCharacterKey('V'); break;
        case CDUKey::Key_W: handleCharacterKey('W'); break;
        case CDUKey::Key_X: handleCharacterKey('X'); break;
        case CDUKey::Key_Y: handleCharacterKey('Y'); break;
        case CDUKey::Key_Z: handleCharacterKey('Z'); break;
        case CDUKey::Key_Slash: handleCharacterKey('/'); break;
        case CDUKey::Key_Dot: handleCharacterKey('.'); break;
        case CDUKey::Key_Plus: handleCharacterKey('+'); break;
        case CDUKey::Key_Minus: handleCharacterKey('-'); break;

        case CDUKey::Key_Clear: handleClearKey(); break;
        case CDUKey::Key_Delete: handleDeleteKey(); break;
        case CDUKey::Key_Exec: handleExecKey(); break;
        case CDUKey::Key_Cancel:
            m_currentState = CDUInputState::Idle;
            m_inputBuffer.clear();
            m_cursorPosition = 0;
            break;

        case CDUKey::Key_Left:
        case CDUKey::Key_Right:
        case CDUKey::Key_Up:
        case CDUKey::Key_Down:
        case CDUKey::Key_PageUp:
        case CDUKey::Key_PageDown:
            handleNavigationKey(key);
            break;

        default:
            handleFunctionKey(key);
            break;
    }
}

void CDUInputStateMachine::setInputBuffer(const std::string& buffer) {
    m_inputBuffer = buffer;
    m_cursorPosition = buffer.length();
}

void CDUInputStateMachine::setState(CDUInputState state) {
    m_currentState = state;
    m_inputBuffer.clear();
    m_cursorPosition = 0;
}

void CDUInputStateMachine::clearBuffer() {
    m_inputBuffer.clear();
    m_cursorPosition = 0;
}

void CDUInputStateMachine::handleCharacterKey(char c) {
    if (m_cursorPosition < 24) {
        m_inputBuffer.insert(m_cursorPosition, 1, c);
        m_cursorPosition++;
    }
}

void CDUInputStateMachine::handleNumericKey(int digit) {
    if (m_cursorPosition < 24) {
        char c = static_cast<char>('0' + digit);
        m_inputBuffer.insert(m_cursorPosition, 1, c);
        m_cursorPosition++;
    }
}

void CDUInputStateMachine::handleClearKey() {
    if (!m_inputBuffer.empty()) {
        m_inputBuffer.clear();
        m_cursorPosition = 0;
    }
}

void CDUInputStateMachine::handleDeleteKey() {
    if (!m_inputBuffer.empty() && m_cursorPosition > 0) {
        m_inputBuffer.erase(m_cursorPosition - 1, 1);
        m_cursorPosition--;
    }
}

void CDUInputStateMachine::handleExecKey() {
    if (!m_inputBuffer.empty()) {
        m_currentState = CDUInputState::ReviewingPlan;
    }
}

void CDUInputStateMachine::handleNavigationKey(CDUKey key) {
    if (key == CDUKey::Key_Left && m_cursorPosition > 0) {
        m_cursorPosition--;
    } else if (key == CDUKey::Key_Right && m_cursorPosition < m_inputBuffer.length()) {
        m_cursorPosition++;
    }
}

void CDUInputStateMachine::handleFunctionKey(CDUKey key) {
    switch (key) {
        case CDUKey::Key_Origin:
            m_currentState = CDUInputState::EnteringOrigin;
            m_inputBuffer.clear();
            m_cursorPosition = 0;
            break;
        case CDUKey::Key_Dest:
            m_currentState = CDUInputState::EnteringDestination;
            m_inputBuffer.clear();
            m_cursorPosition = 0;
            break;
        case CDUKey::Key_Alt:
            m_currentState = CDUInputState::EnteringAlternate;
            m_inputBuffer.clear();
            m_cursorPosition = 0;
            break;
        case CDUKey::Key_Route:
            m_currentState = CDUInputState::EnteringRoute;
            m_inputBuffer.clear();
            m_cursorPosition = 0;
            break;
        case CDUKey::Key_Legs:
        case CDUKey::Key_Prog:
            m_currentState = CDUInputState::ReviewingPlan;
            break;
        default:
            break;
    }
}

std::string CDUInputStateMachine::getStateName() const {
    switch (m_currentState) {
        case CDUInputState::Idle: return "IDLE";
        case CDUInputState::EnteringOrigin: return "ENTERING ORIGIN";
        case CDUInputState::EnteringDestination: return "ENTERING DEST";
        case CDUInputState::EnteringAlternate: return "ENTERING ALTERNATE";
        case CDUInputState::EnteringRoute: return "ENTERING ROUTE";
        case CDUInputState::EnteringAltitude: return "ENTERING ALTITUDE";
        case CDUInputState::EnteringAirway: return "ENTERING AIRWAY";
        case CDUInputState::EnteringSpeed: return "ENTERING SPEED";
        case CDUInputState::ReviewingPlan: return "REVIEWING PLAN";
        case CDUInputState::ExecutingPlan: return "EXECUTING PLAN";
        default: return "UNKNOWN";
    }
}

std::string CDUInputStateMachine::keyToString(CDUKey key) const {
    switch (key) {
        case CDUKey::Key_0: return "0";
        case CDUKey::Key_1: return "1";
        case CDUKey::Key_2: return "2";
        case CDUKey::Key_3: return "3";
        case CDUKey::Key_4: return "4";
        case CDUKey::Key_5: return "5";
        case CDUKey::Key_6: return "6";
        case CDUKey::Key_7: return "7";
        case CDUKey::Key_8: return "8";
        case CDUKey::Key_9: return "9";
        case CDUKey::Key_Dot: return ".";
        case CDUKey::Key_Plus: return "+";
        case CDUKey::Key_Minus: return "-";
        case CDUKey::Key_Slash: return "/";
        case CDUKey::Key_A: return "A";
        case CDUKey::Key_B: return "B";
        case CDUKey::Key_C: return "C";
        case CDUKey::Key_D: return "D";
        case CDUKey::Key_E: return "E";
        case CDUKey::Key_F: return "F";
        case CDUKey::Key_G: return "G";
        case CDUKey::Key_H: return "H";
        case CDUKey::Key_I: return "I";
        case CDUKey::Key_J: return "J";
        case CDUKey::Key_K: return "K";
        case CDUKey::Key_L: return "L";
        case CDUKey::Key_M: return "M";
        case CDUKey::Key_N: return "N";
        case CDUKey::Key_O: return "O";
        case CDUKey::Key_P: return "P";
        case CDUKey::Key_Q: return "Q";
        case CDUKey::Key_R: return "R";
        case CDUKey::Key_S: return "S";
        case CDUKey::Key_T: return "T";
        case CDUKey::Key_U: return "U";
        case CDUKey::Key_V: return "V";
        case CDUKey::Key_W: return "W";
        case CDUKey::Key_X: return "X";
        case CDUKey::Key_Y: return "Y";
        case CDUKey::Key_Z: return "Z";
        default: return "";
    }
}

RoutePlanner::RoutePlanner() {}

void RoutePlanner::setDatabase(const arinc424::ARINC424Database& db) {
    m_database = db;
}

bool RoutePlanner::addRouteEntry(const RouteEntry& entry) {
    m_routeEntries.push_back(entry);
    return true;
}

bool RoutePlanner::insertRouteEntry(size_t position, const RouteEntry& entry) {
    if (position <= m_routeEntries.size()) {
        m_routeEntries.insert(m_routeEntries.begin() + position, entry);
        return true;
    }
    return false;
}

bool RoutePlanner::removeRouteEntry(size_t position) {
    if (position < m_routeEntries.size()) {
        m_routeEntries.erase(m_routeEntries.begin() + position);
        return true;
    }
    return false;
}

void RoutePlanner::clearRoute() {
    m_routeEntries.clear();
    m_origin.clear();
    m_destination.clear();
    m_alternate.clear();
}

std::optional<RouteEntry> RoutePlanner::getEntry(size_t index) const {
    if (index < m_routeEntries.size()) {
        return m_routeEntries[index];
    }
    return std::nullopt;
}

void RoutePlanner::setOrigin(const std::string& origin) {
    m_origin = origin;
}

void RoutePlanner::setDestination(const std::string& dest) {
    m_destination = dest;
}

void RoutePlanner::setAlternate(const std::string& alt) {
    m_alternate = alt;
}

std::optional<arinc424::Coordinates> RoutePlanner::resolveCoordinates(const std::string& identifier) const {
    auto wp = m_database.findWaypoint(identifier);
    if (wp.has_value()) {
        return wp->position;
    }

    auto apt = m_database.findAirport(identifier);
    if (apt.has_value()) {
        return apt->position;
    }

    return std::nullopt;
}

bool RoutePlanner::resolveAirway(const std::string& airwayId, const std::string& from,
                                 const std::string& to,
                                 std::vector<arinc424::AirwaySegment>& path) const {
    auto segments = m_database.findAirwaySegments(airwayId);
    if (segments.empty()) return false;

    std::string current = from;
    while (current != to) {
        auto it = std::find_if(segments.begin(), segments.end(),
            [&current](const arinc424::AirwaySegment& seg) {
                return seg.startWaypointId == current;
            });

        if (it == segments.end()) return false;

        path.push_back(*it);
        current = it->endWaypointId;

        if (path.size() > segments.size()) return false;
    }

    return true;
}

std::vector<std::string> RoutePlanner::validateRoute() const {
    std::vector<std::string> errors;

    if (m_origin.empty()) {
        errors.push_back("Origin airport not specified");
    } else {
        auto coords = resolveCoordinates(m_origin);
        if (!coords.has_value()) {
            errors.push_back("Origin " + m_origin + " not found in database");
        }
    }

    if (m_destination.empty()) {
        errors.push_back("Destination airport not specified");
    } else {
        auto coords = resolveCoordinates(m_destination);
        if (!coords.has_value()) {
            errors.push_back("Destination " + m_destination + " not found in database");
        }
    }

    for (size_t i = 0; i < m_routeEntries.size(); ++i) {
        const auto& entry = m_routeEntries[i];
        if (entry.identifier.empty()) {
            errors.push_back("Route entry " + std::to_string(i + 1) + " has empty identifier");
            continue;
        }
        auto coords = resolveCoordinates(entry.identifier);
        if (!coords.has_value()) {
            errors.push_back("Waypoint " + entry.identifier + " not found in database");
        }
    }

    return errors;
}

FlightPlan RoutePlanner::generateFlightPlan() {
    FlightPlan plan;
    plan.origin = m_origin;
    plan.destination = m_destination;
    plan.alternate = m_alternate;

    auto originCoords = resolveCoordinates(m_origin);
    if (originCoords.has_value()) {
        FlightPlanLeg originLeg;
        originLeg.waypointIdentifier = m_origin;
        originLeg.position = *originCoords;
        originLeg.sequenceNumber = 1;
        originLeg.distanceFromOrigin = 0.0;
        originLeg.cumulativeDistance = 0.0;
        originLeg.trueTrack = 0.0;
        originLeg.groundTrack = 0.0;
        originLeg.remarks = "ORIGIN";
        plan.addLeg(originLeg);
    }

    for (const auto& entry : m_routeEntries) {
        auto coords = resolveCoordinates(entry.identifier);
        if (coords.has_value()) {
            FlightPlanLeg leg;
            leg.waypointIdentifier = entry.identifier;
            leg.position = *coords;
            leg.altitude = entry.altitude;
            leg.airway = entry.airway;
            leg.sequenceNumber = plan.legCount() + 1;
            plan.addLeg(leg);
        }
    }

    auto destCoords = resolveCoordinates(m_destination);
    if (destCoords.has_value()) {
        FlightPlanLeg destLeg;
        destLeg.waypointIdentifier = m_destination;
        destLeg.position = *destCoords;
        destLeg.sequenceNumber = plan.legCount() + 1;
        destLeg.remarks = "DESTINATION";
        plan.addLeg(destLeg);
    }

    plan.recalculateDistances();
    return plan;
}

}
}
