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
#include "../core/arinc424_parser.h"
#include "../core/navigation.h"

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

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();
    void loadDefaultDatabase();
    QString generateSampleARINC424Data() const;

    CDUWidget* m_cduWidget;
    LNAVTable* m_lnavTable;
    QSplitter* m_mainSplitter;
    QLabel* m_statusDatabaseLabel;
    QLabel* m_statusWaypointsLabel;
    QLabel* m_statusDistanceLabel;
    QLabel* m_statusFuelLabel;
    QLabel* m_statusTimeLabel;

    arinc424::ARINC424Database m_database;
    arinc424::ARINC424Parser m_parser;
    nav::FlightPlan m_currentPlan;

    bool m_databaseLoaded;
    bool m_routeExecuted;
    int m_simulatedLegIndex;
    QTimer* m_flightSimTimer;

    static const QString DEFAULT_DATA_PATH;
};

}
}
