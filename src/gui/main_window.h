#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include "cdu_widget.h"
#include "lnav_table.h"
#include "vnav_profile_widget.h"
#include "../core/arinc424_parser.h"
#include "../core/navigation.h"
#include "../core/vnav_solver.h"
#include "../core/bada_aircraft.h"
#include "../core/bada_wind.h"
#include "../core/bada_integrator.h"

namespace fmc {
namespace gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onLoadDatabase();
    void onLoadDefaultRoute();
    void onClearRoute();
    void onExecuteRoute();
    void onExportFlightPlan();
    void onAbout();
    void onFlightPlanChanged(const nav::FlightPlan& plan);
    void onRouteExecuted(const nav::FlightPlan& plan);
    void updateStatusBar();
    void simulateFlightProgress();
    void onVNAVWaypointClicked(size_t legIndex);
    void onEditVNAVConstraints();

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();
    void loadDefaultDatabase();
    QString generateSampleARINC424Data() const;
    void runVNAVSolver();
    void runBADATrajectoryIntegration();
    nav::AltitudeConstraint getWaypointConstraint(size_t legIndex) const;
    void setWaypointConstraint(size_t legIndex, const nav::AltitudeConstraint& c);
    void applyDefaultSTARConstraints();
    void onSelectAircraft();
    void onConfigureWind();
    void onSetCruiseMach();
    void updateTrajectoryDisplay();

    CDUWidget* m_cduWidget;
    LNAVTable* m_lnavTable;
    VNAVProfileWidget* m_vnavWidget;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    QLabel* m_statusDatabaseLabel;
    QLabel* m_statusWaypointsLabel;
    QLabel* m_statusDistanceLabel;
    QLabel* m_statusFuelLabel;
    QLabel* m_statusTimeLabel;
    QLabel* m_statusVNAVLabel;
    QLabel* m_statusAircraftLabel;
    QLabel* m_statusTrajectoryLabel;

    arinc424::ARINC424Database m_database;
    arinc424::ARINC424Parser m_parser;
    nav::FlightPlan m_currentPlan;
    nav::VNAVProfile m_vnavProfile;
    nav::VNAVSolver m_vnavSolver;
    std::vector<nav::AltitudeConstraint> m_altitudeConstraints;

    bada::BADAAircraft m_aircraft;
    bada::WindModel m_windModel;
    bada::BADATrajectoryIntegrator m_trajectoryIntegrator;
    bada::TrajectoryIntegrationResult m_trajectoryResult;
    double m_cruiseMach;
    double m_initialFuelKg;
    double m_initialMassKg;

    bool m_databaseLoaded;
    bool m_routeExecuted;
    int m_simulatedLegIndex;
    QTimer* m_flightSimTimer;

    static const QString DEFAULT_DATA_PATH;
};

}
}
