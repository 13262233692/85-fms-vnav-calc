#include "arinc424_parser.h"
#include <iostream>
#include <iomanip>

namespace fmc {
namespace arinc424 {

ARINC424Parser::ARINC424Parser() {
    resetStatistics();
}

void ARINC424Parser::resetStatistics() {
    m_statistics.totalRecords = 0;
    m_statistics.waypointRecords = 0;
    m_statistics.navaidRecords = 0;
    m_statistics.airwayRecords = 0;
    m_statistics.airportRecords = 0;
    m_statistics.skippedRecords = 0;
    m_statistics.errorRecords = 0;
    m_errors.clear();
}

std::string ARINC424Parser::trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(static_cast<unsigned char>(*end)));
    return std::string(start, end + 1);
}

std::string ARINC424Parser::extractField(const std::string& record, size_t start, size_t length) {
    if (start + length > record.size()) {
        return std::string();
    }
    return record.substr(start, length);
}

RecordType ARINC424Parser::identifyRecordType(const std::string& record) {
    std::string recordType = trim(extractField(record, FIELD_RECORD_TYPE_START, FIELD_RECORD_TYPE_LEN));
    std::string section = trim(extractField(record, FIELD_SECTION_START, FIELD_SECTION_LEN));
    std::string subsection = trim(extractField(record, FIELD_SUBSECTION_START, FIELD_SUBSECTION_LEN));

    if (section == "P" && subsection == "A") return RecordType::AirportReferencePoint;
    if (section == "P" && subsection == "G") return RecordType::Runway;
    if (section == "E" && subsection == "A") return RecordType::Waypoint;
    if (section == "D") return RecordType::Navaid;
    if (section == "E" && subsection == "R") return RecordType::Airway;
    if (section == "R" || section == "S") return RecordType::Procedure;

    return RecordType::Unsupported;
}

double ARINC424Parser::parseLatitude(const std::string& latStr) {
    if (latStr.size() < 9) return 0.0;

    char hemisphere = latStr[0];
    int degrees = std::stoi(latStr.substr(1, 2));
    int minutes = std::stoi(latStr.substr(3, 2));
    int seconds = std::stoi(latStr.substr(5, 2));
    int milliseconds = std::stoi(latStr.substr(7, 2));

    double decimal = degrees + (minutes / 60.0) + (seconds / 3600.0) + (milliseconds / 360000.0);

    if (hemisphere == 'S' || hemisphere == 's') {
        decimal = -decimal;
    }

    return decimal;
}

double ARINC424Parser::parseLongitude(const std::string& lonStr) {
    if (lonStr.size() < 10) return 0.0;

    char hemisphere = lonStr[0];
    int degrees = std::stoi(lonStr.substr(1, 3));
    int minutes = std::stoi(lonStr.substr(4, 2));
    int seconds = std::stoi(lonStr.substr(6, 2));
    int milliseconds = std::stoi(lonStr.substr(8, 2));

    double decimal = degrees + (minutes / 60.0) + (seconds / 3600.0) + (milliseconds / 360000.0);

    if (hemisphere == 'W' || hemisphere == 'w') {
        decimal = -decimal;
    }

    return decimal;
}

std::optional<double> ARINC424Parser::parseElevation(const std::string& elevStr) {
    std::string trimmed = trim(elevStr);
    if (trimmed.empty() || trimmed.find_first_not_of("0123456789-") != std::string::npos) {
        return std::nullopt;
    }
    try {
        return static_cast<double>(std::stoi(trimmed));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int> ARINC424Parser::parseFrequency(const std::string& freqStr) {
    std::string trimmed = trim(freqStr);
    if (trimmed.empty()) return std::nullopt;
    try {
        int freqKhz = std::stoi(trimmed);
        return freqKhz;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<double> ARINC424Parser::parseBearing(const std::string& bearingStr) {
    std::string trimmed = trim(bearingStr);
    if (trimmed.empty()) return std::nullopt;
    try {
        return std::stod(trimmed);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<double> ARINC424Parser::parseDistance(const std::string& distStr) {
    std::string trimmed = trim(distStr);
    if (trimmed.empty()) return std::nullopt;
    try {
        return std::stod(trimmed);
    } catch (...) {
        return std::nullopt;
    }
}

WaypointType ARINC424Parser::determineWaypointType(const std::string& record) {
    std::string section = trim(extractField(record, FIELD_SECTION_START, FIELD_SECTION_LEN));
    std::string subsection = trim(extractField(record, FIELD_SUBSECTION_START, FIELD_SUBSECTION_LEN));

    if (section == "E" && subsection == "A") return WaypointType::Enroute;
    if (section == "P" && subsection == "A") return WaypointType::Airport;

    return WaypointType::Unknown;
}

Waypoint ARINC424Parser::parseWaypointRecord(const std::string& record) {
    Waypoint wp;
    wp.identifier = trim(extractField(record, FIELD_IDENTIFIER_START, FIELD_IDENTIFIER_LEN));
    wp.regionCode = trim(extractField(record, FIELD_AREA_CODE_START, FIELD_AREA_CODE_LEN));
    wp.icaoCode = trim(extractField(record, FIELD_ICAO_START, FIELD_ICAO_LEN));

    std::string latStr = extractField(record, FIELD_LATITUDE_START, FIELD_LATITUDE_LEN);
    std::string lonStr = extractField(record, FIELD_LONGITUDE_START, FIELD_LONGITUDE_LEN);
    wp.position = Coordinates(parseLatitude(latStr), parseLongitude(lonStr));

    std::string elevStr = extractField(record, FIELD_ELEVATION_START, FIELD_ELEVATION_LEN);
    wp.elevation = parseElevation(elevStr);

    wp.type = determineWaypointType(record);
    wp.magneticVariation = trim(extractField(record, FIELD_MAGVAR_START, FIELD_MAGVAR_LEN));

    return wp;
}

Navaid ARINC424Parser::parseNavaidRecord(const std::string& record) {
    Navaid nav;
    nav.identifier = trim(extractField(record, FIELD_IDENTIFIER_START, FIELD_IDENTIFIER_LEN));
    nav.regionCode = trim(extractField(record, FIELD_AREA_CODE_START, FIELD_AREA_CODE_LEN));
    nav.icaoCode = trim(extractField(record, FIELD_ICAO_START, FIELD_ICAO_LEN));

    std::string latStr = extractField(record, FIELD_LATITUDE_START, FIELD_LATITUDE_LEN);
    std::string lonStr = extractField(record, FIELD_LONGITUDE_START, FIELD_LONGITUDE_LEN);
    nav.position = Coordinates(parseLatitude(latStr), parseLongitude(lonStr));

    std::string elevStr = extractField(record, FIELD_ELEVATION_START, FIELD_ELEVATION_LEN);
    nav.elevation = parseElevation(elevStr);

    std::string freqStr = extractField(record, FIELD_FREQ_START, FIELD_FREQ_LEN);
    nav.frequency = parseFrequency(freqStr);

    nav.type = trim(extractField(record, FIELD_SUBSECTION_START, FIELD_SUBSECTION_LEN));

    return nav;
}

AirwaySegment ARINC424Parser::parseAirwayRecord(const std::string& record) {
    AirwaySegment seg;
    seg.airwayIdentifier = trim(extractField(record, FIELD_AIRWAY_ID_START, FIELD_AIRWAY_ID_LEN));
    seg.startWaypointId = trim(extractField(record, FIELD_IDENTIFIER_START, FIELD_IDENTIFIER_LEN));
    seg.regionCode = trim(extractField(record, FIELD_AREA_CODE_START, FIELD_AREA_CODE_LEN));

    std::string bearingStr = extractField(record, FIELD_BEARING_START, FIELD_BEARING_LEN);
    seg.outboundBearing = parseBearing(bearingStr);

    std::string distStr = extractField(record, FIELD_DISTANCE_START, FIELD_DISTANCE_LEN);
    seg.distance = parseDistance(distStr);

    seg.directionRestriction = trim(extractField(record, 64, 1));

    size_t endWpStart = 100;
    size_t endWpLen = 5;
    seg.endWaypointId = trim(extractField(record, endWpStart, endWpLen));

    return seg;
}

Airport ARINC424Parser::parseAirportRecord(const std::string& record) {
    Airport apt;
    apt.icaoCode = trim(extractField(record, FIELD_IDENTIFIER_START, FIELD_IDENTIFIER_LEN));
    apt.regionCode = trim(extractField(record, FIELD_AREA_CODE_START, FIELD_AREA_CODE_LEN));
    apt.name = trim(extractField(record, FIELD_NAME_START, FIELD_NAME_LEN));

    std::string latStr = extractField(record, FIELD_LATITUDE_START, FIELD_LATITUDE_LEN);
    std::string lonStr = extractField(record, FIELD_LONGITUDE_START, FIELD_LONGITUDE_LEN);
    apt.position = Coordinates(parseLatitude(latStr), parseLongitude(lonStr));

    std::string elevStr = extractField(record, FIELD_ELEVATION_START, FIELD_ELEVATION_LEN);
    apt.elevation = parseElevation(elevStr);

    apt.transitionAltitude = trim(extractField(record, 84, 4));

    return apt;
}

void ARINC424Parser::parseRecord(const std::string& record, ARINC424Database& db) {
    m_statistics.totalRecords++;

    if (record.size() < ARINC424_RECORD_LENGTH) {
        m_statistics.errorRecords++;
        m_errors.push_back("Record too short: " + std::to_string(record.size()) + " chars");
        return;
    }

    try {
        RecordType type = identifyRecordType(record);

        switch (type) {
            case RecordType::Waypoint: {
                Waypoint wp = parseWaypointRecord(record);
                if (!wp.identifier.empty()) {
                    db.waypoints.push_back(wp);
                    m_statistics.waypointRecords++;
                }
                break;
            }
            case RecordType::Navaid: {
                Navaid nav = parseNavaidRecord(record);
                if (!nav.identifier.empty()) {
                    db.navaids.push_back(nav);
                    m_statistics.navaidRecords++;
                }
                break;
            }
            case RecordType::Airway: {
                AirwaySegment seg = parseAirwayRecord(record);
                if (!seg.airwayIdentifier.empty() && !seg.startWaypointId.empty()) {
                    db.airwaySegments.push_back(seg);
                    m_statistics.airwayRecords++;
                }
                break;
            }
            case RecordType::AirportReferencePoint: {
                Airport apt = parseAirportRecord(record);
                if (!apt.icaoCode.empty()) {
                    db.airports.push_back(apt);
                    m_statistics.airportRecords++;
                }
                break;
            }
            default:
                m_statistics.skippedRecords++;
                break;
        }
    } catch (const std::exception& e) {
        m_statistics.errorRecords++;
        m_errors.push_back("Parse error: " + std::string(e.what()));
    }
}

ARINC424Database ARINC424Parser::parseString(const std::string& content) {
    resetStatistics();
    ARINC424Database db;

    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) continue;
        parseRecord(line, db);
    }

    return db;
}

ARINC424Database ARINC424Parser::parseFile(const std::string& filePath) {
    resetStatistics();
    ARINC424Database db;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        m_errors.push_back("Failed to open file: " + filePath);
        return db;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) continue;
        parseRecord(line, db);
    }

    file.close();
    return db;
}

std::optional<Waypoint> ARINC424Database::findWaypoint(const std::string& identifier) const {
    auto it = std::find_if(waypoints.begin(), waypoints.end(),
        [&identifier](const Waypoint& wp) { return wp.identifier == identifier; });
    if (it != waypoints.end()) {
        return *it;
    }
    return std::nullopt;
}

std::optional<Airport> ARINC424Database::findAirport(const std::string& icaoCode) const {
    auto it = std::find_if(airports.begin(), airports.end(),
        [&icaoCode](const Airport& apt) { return apt.icaoCode == icaoCode; });
    if (it != airports.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<AirwaySegment> ARINC424Database::findAirwaySegments(const std::string& airwayId) const {
    std::vector<AirwaySegment> results;
    std::copy_if(airwaySegments.begin(), airwaySegments.end(), std::back_inserter(results),
        [&airwayId](const AirwaySegment& seg) { return seg.airwayIdentifier == airwayId; });
    return results;
}

std::vector<AirwaySegment> ARINC424Database::findSegmentsFromWaypoint(const std::string& waypointId) const {
    std::vector<AirwaySegment> results;
    std::copy_if(airwaySegments.begin(), airwaySegments.end(), std::back_inserter(results),
        [&waypointId](const AirwaySegment& seg) { return seg.startWaypointId == waypointId; });
    return results;
}

}
}
