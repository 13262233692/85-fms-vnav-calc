#pragma once

#include <string>
#include <vector>
#include <array>
#include <optional>
#include <cmath>

namespace fmc {
namespace arinc424 {

constexpr size_t ARINC424_RECORD_LENGTH = 132;

enum class RecordType {
    AirportReferencePoint = 0,
    Runway = 1,
    Waypoint = 2,
    Navaid = 3,
    Airway = 4,
    Procedure = 5,
    Unsupported = 99
};

enum class WaypointType {
    Enroute,
    Terminal,
    Airport,
    RunwayThreshold,
    MissedApproachPoint,
    HoldingFix,
    Unknown
};

struct Coordinates {
    double latitude;
    double longitude;

    Coordinates() : latitude(0.0), longitude(0.0) {}
    Coordinates(double lat, double lon) : latitude(lat), longitude(lon) {}
};

struct Waypoint {
    std::string identifier;
    std::string regionCode;
    std::string icaoCode;
    Coordinates position;
    std::optional<double> elevation;
    WaypointType type;
    std::string magneticVariation;
    std::string description;

    Waypoint() : type(WaypointType::Unknown) {}
};

struct Navaid {
    std::string identifier;
    std::string regionCode;
    std::string icaoCode;
    Coordinates position;
    std::optional<double> elevation;
    std::string type;
    std::optional<int> frequency;
    std::optional<double> range;
    std::optional<double> magneticVariation;
};

struct AirwaySegment {
    std::string airwayIdentifier;
    std::string startWaypointId;
    std::string endWaypointId;
    std::optional<double> outboundBearing;
    std::optional<double> inboundBearing;
    std::optional<double> distance;
    std::string regionCode;
    std::string directionRestriction;
};

struct Airport {
    std::string icaoCode;
    std::string regionCode;
    std::string name;
    Coordinates position;
    std::optional<double> elevation;
    std::optional<double> magneticVariation;
    std::string transitionAltitude;
};

struct ARINC424Database {
    std::vector<Waypoint> waypoints;
    std::vector<Navaid> navaids;
    std::vector<AirwaySegment> airwaySegments;
    std::vector<Airport> airports;

    std::optional<Waypoint> findWaypoint(const std::string& identifier) const;
    std::optional<Airport> findAirport(const std::string& icaoCode) const;
    std::vector<AirwaySegment> findAirwaySegments(const std::string& airwayId) const;
    std::vector<AirwaySegment> findSegmentsFromWaypoint(const std::string& waypointId) const;
};

}
}
