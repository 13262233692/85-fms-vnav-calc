#pragma once

#include "arinc424_types.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace fmc {
namespace arinc424 {

class ARINC424Parser {
public:
    ARINC424Parser();
    ~ARINC424Parser() = default;

    ARINC424Database parseFile(const std::string& filePath);
    ARINC424Database parseString(const std::string& content);

    struct ParseStatistics {
        size_t totalRecords;
        size_t waypointRecords;
        size_t navaidRecords;
        size_t airwayRecords;
        size_t airportRecords;
        size_t skippedRecords;
        size_t errorRecords;
    };

    const ParseStatistics& getStatistics() const { return m_statistics; }
    const std::vector<std::string>& getErrors() const { return m_errors; }

private:
    ParseStatistics m_statistics;
    std::vector<std::string> m_errors;

    void resetStatistics();
    void parseRecord(const std::string& record, ARINC424Database& db);
    RecordType identifyRecordType(const std::string& record);

    Waypoint parseWaypointRecord(const std::string& record);
    Navaid parseNavaidRecord(const std::string& record);
    AirwaySegment parseAirwayRecord(const std::string& record);
    Airport parseAirportRecord(const std::string& record);

    std::string extractField(const std::string& record, size_t start, size_t length);
    std::string trim(const std::string& s);

    double parseLatitude(const std::string& latStr);
    double parseLongitude(const std::string& lonStr);
    std::optional<double> parseElevation(const std::string& elevStr);
    std::optional<int> parseFrequency(const std::string& freqStr);
    std::optional<double> parseBearing(const std::string& bearingStr);
    std::optional<double> parseDistance(const std::string& distStr);

    WaypointType determineWaypointType(const std::string& record);

    static constexpr size_t FIELD_RECORD_TYPE_START = 0;
    static constexpr size_t FIELD_RECORD_TYPE_LEN = 3;
    static constexpr size_t FIELD_CUSTOMER_CODE_START = 3;
    static constexpr size_t FIELD_CUSTOMER_CODE_LEN = 2;
    static constexpr size_t FIELD_SECTION_START = 5;
    static constexpr size_t FIELD_SECTION_LEN = 1;
    static constexpr size_t FIELD_SUBSECTION_START = 6;
    static constexpr size_t FIELD_SUBSECTION_LEN = 1;
    static constexpr size_t FIELD_AREA_CODE_START = 12;
    static constexpr size_t FIELD_AREA_CODE_LEN = 3;
    static constexpr size_t FIELD_ICAO_START = 19;
    static constexpr size_t FIELD_ICAO_LEN = 4;
    static constexpr size_t FIELD_IDENTIFIER_START = 23;
    static constexpr size_t FIELD_IDENTIFIER_LEN = 5;
    static constexpr size_t FIELD_LATITUDE_START = 32;
    static constexpr size_t FIELD_LATITUDE_LEN = 9;
    static constexpr size_t FIELD_LONGITUDE_START = 41;
    static constexpr size_t FIELD_LONGITUDE_LEN = 10;
    static constexpr size_t FIELD_ELEVATION_START = 55;
    static constexpr size_t FIELD_ELEVATION_LEN = 6;
    static constexpr size_t FIELD_AIRWAY_ID_START = 14;
    static constexpr size_t FIELD_AIRWAY_ID_LEN = 5;
    static constexpr size_t FIELD_FREQ_START = 40;
    static constexpr size_t FIELD_FREQ_LEN = 5;
    static constexpr size_t FIELD_MAGVAR_START = 74;
    static constexpr size_t FIELD_MAGVAR_LEN = 5;
    static constexpr size_t FIELD_BEARING_START = 66;
    static constexpr size_t FIELD_BEARING_LEN = 3;
    static constexpr size_t FIELD_DISTANCE_START = 69;
    static constexpr size_t FIELD_DISTANCE_LEN = 3;
    static constexpr size_t FIELD_NAME_START = 94;
    static constexpr size_t FIELD_NAME_LEN = 30;
};

}
}
