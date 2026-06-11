#include "lnav_table.h"
#include <QTableWidgetItem>
#include <QFontDatabase>
#include <QHeaderView>
#include <QScrollBar>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace fmc {
namespace gui {

const QColor LNAVTable::HEADER_BG_COLOR(20, 30, 48);
const QColor LNAVTable::ACTIVE_LEG_COLOR(255, 200, 0, 40);
const QColor LNAVTable::ALTERNATE_ROW_COLOR(30, 40, 60);
const QColor LNAVTable::TEXT_COLOR(220, 220, 220);
const QColor LNAVTable::DISTANCE_COLOR(0, 200, 255);
const QColor LNAVTable::BEARING_COLOR(0, 255, 150);
const QColor LNAVTable::ALTITUDE_COLOR(255, 180, 100);

LNAVTable::LNAVTable(QWidget* parent)
    : QTableWidget(parent),
      m_magneticVariation(0.0),
      m_activeLegIndex(-1),
      m_hasValidPlan(false),
      m_hasTrajectoryData(false) {
    setupTable();
}

void LNAVTable::setupTable() {
    setColumnCount(COL_COUNT);
    setRowCount(0);

    QStringList headers;
    headers << "SEQ" << "WAYPOINT" << "AIRWAY" << "LATITUDE" << "LONGITUDE"
            << "ALT" << "DIST" << "CUM" << "TRK(TRUE)" << "TRK(MAG)"
            << "TAS" << "GS" << "MACH" << "V/S"
            << "EFOB" << "FF" << "ETA" << "REMARKS";
    setHorizontalHeaderLabels(headers);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(70);
    setColumnWidth(COL_SEQUENCE, 50);
    setColumnWidth(COL_WAYPOINT, 90);
    setColumnWidth(COL_AIRWAY, 75);
    setColumnWidth(COL_LATITUDE, 110);
    setColumnWidth(COL_LONGITUDE, 110);
    setColumnWidth(COL_ALTITUDE, 65);
    setColumnWidth(COL_DISTANCE, 65);
    setColumnWidth(COL_CUMULATIVE, 75);
    setColumnWidth(COL_TRUE_TRACK, 80);
    setColumnWidth(COL_MAG_TRACK, 80);
    setColumnWidth(COL_TAS, 65);
    setColumnWidth(COL_GS, 65);
    setColumnWidth(COL_MACH, 60);
    setColumnWidth(COL_VS, 70);
    setColumnWidth(COL_EFOB, 80);
    setColumnWidth(COL_FF, 75);
    setColumnWidth(COL_ETA, 65);
    setColumnWidth(COL_REMARKS, 120);

    verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
    verticalHeader()->setVisible(false);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);

    setStyleSheet(R"(
        QTableWidget {
            background-color: rgb(15, 20, 35);
            gridline-color: rgb(60, 70, 90);
            color: rgb(220, 220, 220);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 11px;
            border: 1px solid rgb(60, 70, 90);
        }
        QHeaderView::section {
            background-color: rgb(20, 30, 48);
            color: rgb(0, 200, 255);
            padding: 6px;
            border: none;
            border-right: 1px solid rgb(60, 70, 90);
            border-bottom: 1px solid rgb(60, 70, 90);
            font-weight: bold;
            font-size: 10px;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QTableWidget::item:selected {
            background-color: rgba(255, 200, 0, 60);
            color: white;
        }
        QScrollBar:vertical {
            background-color: rgb(20, 30, 48);
            width: 12px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: rgb(60, 80, 120);
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    horizontalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            background-color: rgb(20, 30, 48);
            color: rgb(0, 200, 255);
            padding: 6px;
            border: none;
            border-right: 1px solid rgb(60, 70, 90);
            border-bottom: 1px solid rgb(60, 70, 90);
            font-weight: bold;
            font-size: 10px;
        }
    )");
}

void LNAVTable::setFlightPlan(const nav::FlightPlan& plan) {
    m_flightPlan = plan;
    m_hasValidPlan = !plan.legs.empty();
    m_activeLegIndex = -1;
    populateTable();
}

void LNAVTable::updateFlightPlan(const nav::FlightPlan& plan) {
    m_flightPlan = plan;
    m_hasValidPlan = !plan.legs.empty();
    populateTable();
}

void LNAVTable::setTrajectoryResult(const bada::TrajectoryIntegrationResult& result) {
    m_trajectory = result;
    m_hasTrajectoryData = result.success && !result.legs.empty();
    if (m_hasValidPlan) {
        populateTable();
    }
}

void LNAVTable::clearFlightPlan() {
    m_flightPlan.clear();
    m_trajectory = bada::TrajectoryIntegrationResult();
    m_hasValidPlan = false;
    m_hasTrajectoryData = false;
    m_activeLegIndex = -1;
    setRowCount(0);
}

void LNAVTable::highlightLeg(size_t legIndex) {
    if (legIndex >= static_cast<size_t>(rowCount())) return;

    for (int row = 0; row < rowCount(); ++row) {
        QBrush bgBrush = (row % 2 == 0) ? QBrush(QColor(15, 20, 35)) : QBrush(ALTERNATE_ROW_COLOR);
        for (int col = 0; col < columnCount(); ++col) {
            QTableWidgetItem* item = this->item(row, col);
            if (item) {
                item->setBackground(bgBrush);
            }
        }
    }

    for (int col = 0; col < columnCount(); ++col) {
        QTableWidgetItem* item = this->item(static_cast<int>(legIndex), col);
        if (item) {
            item->setBackground(QBrush(ACTIVE_LEG_COLOR));
        }
    }
}

void LNAVTable::selectLeg(size_t legIndex) {
    if (legIndex < static_cast<size_t>(rowCount())) {
        selectRow(static_cast<int>(legIndex));
        m_activeLegIndex = static_cast<int>(legIndex);
    }
}

std::optional<size_t> LNAVTable::getSelectedLegIndex() const {
    QList<QTableWidgetItem*> selected = selectedItems();
    if (selected.isEmpty()) return std::nullopt;
    return static_cast<size_t>(selected.first()->row());
}

void LNAVTable::refreshDisplay() {
    if (m_hasValidPlan) {
        populateTable();
    }
}

void LNAVTable::updateActiveLeg(int legIndex) {
    m_activeLegIndex = legIndex;
    if (legIndex >= 0 && legIndex < rowCount()) {
        highlightLeg(static_cast<size_t>(legIndex));
    }
}

void LNAVTable::populateTable() {
    setRowCount(0);

    if (!m_hasValidPlan) return;

    setRowCount(static_cast<int>(m_flightPlan.legs.size()));

    for (size_t i = 0; i < m_flightPlan.legs.size(); ++i) {
        const auto& leg = m_flightPlan.legs[i];
        int row = static_cast<int>(i);

        QBrush bgBrush = (row % 2 == 0) ? QBrush(QColor(15, 20, 35)) : QBrush(ALTERNATE_ROW_COLOR);
        if (row == m_activeLegIndex) {
            bgBrush = QBrush(ACTIVE_LEG_COLOR);
        }

        QTableWidgetItem* seqItem = new QTableWidgetItem(QString::number(leg.sequenceNumber));
        seqItem->setTextAlignment(Qt::AlignCenter);
        seqItem->setForeground(QBrush(QColor(255, 255, 255)));
        seqItem->setBackground(bgBrush);
        setItem(row, COL_SEQUENCE, seqItem);

        QTableWidgetItem* wpItem = new QTableWidgetItem(QString::fromStdString(leg.waypointIdentifier));
        wpItem->setTextAlignment(Qt::AlignCenter);
        wpItem->setForeground(QBrush(TEXT_COLOR));
        wpItem->setBackground(bgBrush);
        QFont wpFont = wpItem->font();
        wpFont.setBold(true);
        wpItem->setFont(wpFont);
        setItem(row, COL_WAYPOINT, wpItem);

        QTableWidgetItem* airwayItem = new QTableWidgetItem(QString::fromStdString(leg.airway));
        airwayItem->setTextAlignment(Qt::AlignCenter);
        airwayItem->setForeground(QBrush(QColor(200, 150, 255)));
        airwayItem->setBackground(bgBrush);
        setItem(row, COL_AIRWAY, airwayItem);

        QTableWidgetItem* latItem = new QTableWidgetItem(formatLatitude(leg.position.latitude));
        latItem->setTextAlignment(Qt::AlignCenter);
        latItem->setForeground(QBrush(TEXT_COLOR));
        latItem->setBackground(bgBrush);
        setItem(row, COL_LATITUDE, latItem);

        QTableWidgetItem* lonItem = new QTableWidgetItem(formatLongitude(leg.position.longitude));
        lonItem->setTextAlignment(Qt::AlignCenter);
        lonItem->setForeground(QBrush(TEXT_COLOR));
        lonItem->setBackground(bgBrush);
        setItem(row, COL_LONGITUDE, lonItem);

        QString altText = leg.altitude.has_value() ? formatAltitude(*leg.altitude) : "--";
        QTableWidgetItem* altItem = new QTableWidgetItem(altText);
        altItem->setTextAlignment(Qt::AlignCenter);
        altItem->setForeground(QBrush(ALTITUDE_COLOR));
        altItem->setBackground(bgBrush);
        setItem(row, COL_ALTITUDE, altItem);

        QString distText = (i > 0) ? formatDistance(leg.distanceFromOrigin) : "--";
        QTableWidgetItem* distItem = new QTableWidgetItem(distText);
        distItem->setTextAlignment(Qt::AlignCenter);
        distItem->setForeground(QBrush(DISTANCE_COLOR));
        distItem->setBackground(bgBrush);
        setItem(row, COL_DISTANCE, distItem);

        QString cumText = formatDistance(leg.cumulativeDistance);
        QTableWidgetItem* cumItem = new QTableWidgetItem(cumText);
        cumItem->setTextAlignment(Qt::AlignCenter);
        cumItem->setForeground(QBrush(DISTANCE_COLOR));
        cumItem->setBackground(bgBrush);
        setItem(row, COL_CUMULATIVE, cumItem);

        QString trkText = (i > 0) ? formatBearing(leg.trueTrack) : "--";
        QTableWidgetItem* trkItem = new QTableWidgetItem(trkText);
        trkItem->setTextAlignment(Qt::AlignCenter);
        trkItem->setForeground(QBrush(BEARING_COLOR));
        trkItem->setBackground(bgBrush);
        setItem(row, COL_TRUE_TRACK, trkItem);

        double magTrack = std::fmod(leg.trueTrack - m_magneticVariation + 360.0, 360.0);
        QString magTrkText = (i > 0) ? formatBearing(magTrack) : "--";
        QTableWidgetItem* magTrkItem = new QTableWidgetItem(magTrkText);
        magTrkItem->setTextAlignment(Qt::AlignCenter);
        magTrkItem->setForeground(QBrush(BEARING_COLOR));
        magTrkItem->setBackground(bgBrush);
        setItem(row, COL_MAG_TRACK, magTrkItem);

        QString tasText = "--";
        QString gsText = "--";
        QString machText = "--";
        QString vsText = "--";
        QString efobText = "--";
        QString ffText = "--";
        QString etaText = leg.estimatedTimeEnroute.has_value()
                              ? formatETA(*leg.estimatedTimeEnroute)
                              : "--";

        if (m_hasTrajectoryData && i < m_trajectory.legs.size()) {
            const auto& tjLeg = m_trajectory.legs[i];
            tasText = formatTAS(tjLeg.finalTasKT);
            gsText = formatGS(tjLeg.avgGsKT);
            machText = formatMach(tjLeg.finalMach);
            vsText = formatVS(tjLeg.avgVerticalSpeedFpm);
            efobText = formatEFOB(tjLeg.fuelRemainingKg);
            ffText = formatFuelFlow(tjLeg.avgFuelFlowKgMin);
            etaText = formatETA(tjLeg.cumulativeTimeMin);
        }

        QTableWidgetItem* tasItem = new QTableWidgetItem(tasText);
        tasItem->setTextAlignment(Qt::AlignCenter);
        tasItem->setForeground(QBrush(QColor(100, 255, 255)));
        tasItem->setBackground(bgBrush);
        setItem(row, COL_TAS, tasItem);

        QTableWidgetItem* gsItem = new QTableWidgetItem(gsText);
        gsItem->setTextAlignment(Qt::AlignCenter);
        gsItem->setForeground(QBrush(QColor(0, 255, 200)));
        gsItem->setBackground(bgBrush);
        setItem(row, COL_GS, gsItem);

        QTableWidgetItem* machItem = new QTableWidgetItem(machText);
        machItem->setTextAlignment(Qt::AlignCenter);
        machItem->setForeground(QBrush(QColor(180, 220, 255)));
        machItem->setBackground(bgBrush);
        setItem(row, COL_MACH, machItem);

        QTableWidgetItem* vsItem = new QTableWidgetItem(vsText);
        vsItem->setTextAlignment(Qt::AlignCenter);
        vsItem->setForeground(QBrush(QColor(255, 180, 100)));
        vsItem->setBackground(bgBrush);
        setItem(row, COL_VS, vsItem);

        QTableWidgetItem* efobItem = new QTableWidgetItem(efobText);
        efobItem->setTextAlignment(Qt::AlignCenter);
        efobItem->setForeground(QBrush(QColor(255, 220, 100)));
        efobItem->setBackground(bgBrush);
        setItem(row, COL_EFOB, efobItem);

        QTableWidgetItem* ffItem = new QTableWidgetItem(ffText);
        ffItem->setTextAlignment(Qt::AlignCenter);
        ffItem->setForeground(QBrush(QColor(255, 150, 50)));
        ffItem->setBackground(bgBrush);
        setItem(row, COL_FF, ffItem);

        QTableWidgetItem* etaItem = new QTableWidgetItem(etaText);
        etaItem->setTextAlignment(Qt::AlignCenter);
        etaItem->setForeground(QBrush(QColor(255, 220, 100)));
        etaItem->setBackground(bgBrush);
        setItem(row, COL_ETA, etaItem);

        QTableWidgetItem* remarksItem = new QTableWidgetItem(QString::fromStdString(leg.remarks));
        remarksItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        remarksItem->setForeground(QBrush(TEXT_COLOR));
        remarksItem->setBackground(bgBrush);
        setItem(row, COL_REMARKS, remarksItem);

        setRowHeight(row, ROW_HEIGHT);
    }
}

QString LNAVTable::formatTAS(double tasKT) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.0fKT", tasKT);
    return QString(buffer);
}

QString LNAVTable::formatGS(double gsKT) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.0fKT", gsKT);
    return QString(buffer);
}

QString LNAVTable::formatMach(double mach) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "M%.3f", mach);
    return QString(buffer);
}

QString LNAVTable::formatVS(double vsFpm) const {
    char buffer[16];
    if (std::abs(vsFpm) < 50.0) {
        std::snprintf(buffer, sizeof(buffer), "LVL");
    } else {
        std::snprintf(buffer, sizeof(buffer), "%+.0f", vsFpm);
    }
    return QString(buffer);
}

QString LNAVTable::formatEFOB(double fuelKg) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.0fKG", fuelKg);
    return QString(buffer);
}

QString LNAVTable::formatFuelFlow(double ffKgMin) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.1f", ffKgMin * 60.0 / 1000.0);
    return QString(buffer) + "T";
}

QString LNAVTable::formatLatitude(double lat) const {
    char hemisphere = (lat >= 0) ? 'N' : 'S';
    double absLat = std::abs(lat);
    int degrees = static_cast<int>(absLat);
    double minutesDecimal = (absLat - degrees) * 60.0;
    int minutes = static_cast<int>(minutesDecimal);
    double seconds = (minutesDecimal - minutes) * 60.0;

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%c%03d°%02d'%05.2f\"",
                  hemisphere, degrees, minutes, seconds);
    return QString(buffer);
}

QString LNAVTable::formatLongitude(double lon) const {
    char hemisphere = (lon >= 0) ? 'E' : 'W';
    double absLon = std::abs(lon);
    int degrees = static_cast<int>(absLon);
    double minutesDecimal = (absLon - degrees) * 60.0;
    int minutes = static_cast<int>(minutesDecimal);
    double seconds = (minutesDecimal - minutes) * 60.0;

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%c%03d°%02d'%05.2f\"",
                  hemisphere, degrees, minutes, seconds);
    return QString(buffer);
}

QString LNAVTable::formatDistance(double distanceNm) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.1f NM", distanceNm);
    return QString(buffer);
}

QString LNAVTable::formatBearing(double bearing) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%03d°", static_cast<int>(std::round(bearing)));
    return QString(buffer);
}

QString LNAVTable::formatAltitude(double altitudeFt) const {
    if (altitudeFt >= 18000) {
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "FL%03d", static_cast<int>(altitudeFt / 100));
        return QString(buffer);
    } else {
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%d FT", static_cast<int>(altitudeFt));
        return QString(buffer);
    }
}

QString LNAVTable::formatETA(double minutes) const {
    int hours = static_cast<int>(minutes / 60);
    int mins = static_cast<int>(std::fmod(minutes, 60));
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, mins);
    return QString(buffer);
}

}
}
