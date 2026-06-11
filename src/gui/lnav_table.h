#pragma once

#include <QTableWidget>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include "../core/navigation.h"

namespace fmc {
namespace gui {

class LNAVTable : public QTableWidget {
    Q_OBJECT

public:
    explicit LNAVTable(QWidget* parent = nullptr);
    ~LNAVTable() override = default;

    void setFlightPlan(const nav::FlightPlan& plan);
    void updateFlightPlan(const nav::FlightPlan& plan);
    void clearFlightPlan();

    void highlightLeg(size_t legIndex);
    void selectLeg(size_t legIndex);
    std::optional<size_t> getSelectedLegIndex() const;

    void setMagneticVariation(double magVar) { m_magneticVariation = magVar; }
    double getMagneticVariation() const { return m_magneticVariation; }

    enum ColumnIndex {
        COL_SEQUENCE = 0,
        COL_WAYPOINT = 1,
        COL_AIRWAY = 2,
        COL_LATITUDE = 3,
        COL_LONGITUDE = 4,
        COL_ALTITUDE = 5,
        COL_DISTANCE = 6,
        COL_CUMULATIVE = 7,
        COL_TRUE_TRACK = 8,
        COL_MAG_TRACK = 9,
        COL_ETA = 10,
        COL_REMARKS = 11,
        COL_COUNT = 12
    };

public slots:
    void refreshDisplay();
    void updateActiveLeg(int legIndex);

    QString formatLatitude(double lat) const;
    QString formatLongitude(double lon) const;
    QString formatDistance(double distanceNm) const;
    QString formatBearing(double bearing) const;
    QString formatAltitude(double altitudeFt) const;
    QString formatETA(double minutes) const;

protected:
    void setupTable();
    void populateTable();

private:
    nav::FlightPlan m_flightPlan;
    double m_magneticVariation;
    int m_activeLegIndex;
    bool m_hasValidPlan;

    static constexpr int ROW_HEIGHT = 28;
    static const QColor HEADER_BG_COLOR;
    static const QColor ACTIVE_LEG_COLOR;
    static const QColor ALTERNATE_ROW_COLOR;
    static const QColor TEXT_COLOR;
    static const QColor DISTANCE_COLOR;
    static const QColor BEARING_COLOR;
    static const QColor ALTITUDE_COLOR;
};

}
}
