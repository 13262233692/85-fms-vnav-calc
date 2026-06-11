#pragma once

#include <QTableWidget>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include "../core/navigation.h"
#include "../core/vnav_solver.h"
#include "../core/bada_integrator.h"

namespace fmc {
namespace gui {

class LNAVTable : public QTableWidget {
    Q_OBJECT

public:
    explicit LNAVTable(QWidget* parent = nullptr);
    ~LNAVTable() override = default;

    void setFlightPlan(const nav::FlightPlan& plan);
    void updateFlightPlan(const nav::FlightPlan& plan);
    void setTrajectoryResult(const bada::TrajectoryIntegrationResult& result);
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
        COL_TAS = 10,
        COL_GS = 11,
        COL_MACH = 12,
        COL_VS = 13,
        COL_EFOB = 14,
        COL_FF = 15,
        COL_ETA = 16,
        COL_REMARKS = 17,
        COL_COUNT = 18
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
    QString formatTAS(double tasKT) const;
    QString formatGS(double gsKT) const;
    QString formatMach(double mach) const;
    QString formatVS(double vsFpm) const;
    QString formatEFOB(double fuelKg) const;
    QString formatFuelFlow(double ffKgMin) const;

protected:
    void setupTable();
    void populateTable();

private:
    nav::FlightPlan m_flightPlan;
    bada::TrajectoryIntegrationResult m_trajectory;
    double m_magneticVariation;
    int m_activeLegIndex;
    bool m_hasValidPlan;
    bool m_hasTrajectoryData;

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
