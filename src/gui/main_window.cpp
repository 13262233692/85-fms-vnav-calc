#include "main_window.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

namespace fmc {
namespace gui {

const QString MainWindow::DEFAULT_DATA_PATH = "data/arinc424_sample.txt";

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_cduWidget(nullptr),
      m_lnavTable(nullptr),
      m_vnavWidget(nullptr),
      m_mainSplitter(nullptr),
      m_rightSplitter(nullptr),
      m_statusVNAVLabel(nullptr),
      m_databaseLoaded(false),
      m_routeExecuted(false),
      m_simulatedLegIndex(-1),
      m_flightSimTimer(nullptr) {
    setupUI();
    setupMenuBar();
    setupConnections();

    resize(1400, 900);
    setWindowTitle("FMC - Flight Management Computer Simulation");

    loadDefaultDatabase();
}

MainWindow::~MainWindow() {
    if (m_flightSimTimer) {
        m_flightSimTimer->stop();
    }
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QLabel* titleLabel = new QLabel("✈  FLIGHT MANAGEMENT COMPUTER  ✈  VNAV ENHANCED");
    titleLabel->setStyleSheet(R"(
        QLabel {
            background-color: rgb(10, 20, 40);
            color: rgb(255, 200, 0);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 16px;
            font-weight: bold;
            padding: 12px;
            border-bottom: 2px solid rgb(60, 80, 120);
        }
    )");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setHandleWidth(3);
    m_mainSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background-color: rgb(60, 80, 120);
        }
        QSplitter::handle:hover {
            background-color: rgb(100, 140, 200);
        }
    )");

    m_rightSplitter = new QSplitter(Qt::Vertical, this);
    m_rightSplitter->setHandleWidth(3);

    m_lnavTable = new LNAVTable(this);
    m_lnavTable->setMinimumWidth(700);
    m_lnavTable->setMinimumHeight(250);

    m_vnavWidget = new VNAVProfileWidget(this);
    m_vnavWidget->setMinimumHeight(280);

    m_cduWidget = new CDUWidget(this);
    m_cduWidget->setMinimumWidth(480);

    m_rightSplitter->addWidget(m_lnavTable);
    m_rightSplitter->addWidget(m_vnavWidget);
    m_rightSplitter->setStretchFactor(0, 3);
    m_rightSplitter->setStretchFactor(1, 2);
    m_rightSplitter->setSizes(QList<int>() << 450 << 320);

    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->addWidget(m_cduWidget);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 2);

    mainLayout->addWidget(m_mainSplitter, 1);

    setCentralWidget(centralWidget);

    QStatusBar* statusBar = this->statusBar();
    statusBar->setStyleSheet(R"(
        QStatusBar {
            background-color: rgb(15, 25, 45);
            color: rgb(200, 220, 255);
            border-top: 1px solid rgb(60, 80, 120);
            padding: 4px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 10px;
        }
    )");

    m_statusDatabaseLabel = new QLabel("DB: NOT LOADED");
    m_statusDatabaseLabel->setStyleSheet("color: rgb(255, 100, 100); padding: 0 10px;");
    statusBar->addWidget(m_statusDatabaseLabel);

    m_statusVNAVLabel = new QLabel("VNAV: IDLE");
    m_statusVNAVLabel->setStyleSheet("color: rgb(200, 200, 200); padding: 0 10px;");
    statusBar->addWidget(m_statusVNAVLabel);

    m_statusWaypointsLabel = new QLabel("WPT: 0");
    m_statusWaypointsLabel->setStyleSheet("color: rgb(0, 200, 255); padding: 0 10px;");
    statusBar->addPermanentWidget(m_statusWaypointsLabel);

    m_statusDistanceLabel = new QLabel("DIST: -- NM");
    m_statusDistanceLabel->setStyleSheet("color: rgb(0, 255, 150); padding: 0 10px;");
    statusBar->addPermanentWidget(m_statusDistanceLabel);

    m_statusFuelLabel = new QLabel("FUEL: -- KG");
    m_statusFuelLabel->setStyleSheet("color: rgb(255, 180, 100); padding: 0 10px;");
    statusBar->addPermanentWidget(m_statusFuelLabel);

    m_statusTimeLabel = new QLabel("TIME: --:--");
    m_statusTimeLabel->setStyleSheet("color: rgb(255, 220, 100); padding: 0 10px;");
    statusBar->addPermanentWidget(m_statusTimeLabel);

    m_flightSimTimer = new QTimer(this);
    m_flightSimTimer->setInterval(2000);
    connect(m_flightSimTimer, &QTimer::timeout, this, &MainWindow::simulateFlightProgress);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    menuBar->setStyleSheet(R"(
        QMenuBar {
            background-color: rgb(20, 30, 50);
            color: rgb(220, 230, 255);
            border-bottom: 1px solid rgb(60, 80, 120);
            padding: 4px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 11px;
        }
        QMenuBar::item {
            padding: 6px 12px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background-color: rgb(40, 60, 100);
        }
        QMenu {
            background-color: rgb(25, 35, 55);
            color: rgb(220, 230, 255);
            border: 1px solid rgb(60, 80, 120);
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 24px;
        }
        QMenu::item:selected {
            background-color: rgb(40, 80, 140);
        }
        QMenu::separator {
            height: 1px;
            background-color: rgb(60, 80, 120);
            margin: 4px 8px;
        }
    )");

    QMenu* fileMenu = menuBar->addMenu("&File");

    QAction* loadDbAction = fileMenu->addAction("&Load ARINC 424 Database...");
    loadDbAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(loadDbAction, &QAction::triggered, this, &MainWindow::onLoadDatabase);

    QAction* exportAction = fileMenu->addAction("&Export Flight Plan...");
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportFlightPlan);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* routeMenu = menuBar->addMenu("&Route");

    QAction* loadDefaultAction = routeMenu->addAction("&Load Default Route");
    loadDefaultAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(loadDefaultAction, &QAction::triggered, this, &MainWindow::onLoadDefaultRoute);

    QAction* clearRouteAction = routeMenu->addAction("C&lear Route");
    clearRouteAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(clearRouteAction, &QAction::triggered, this, &MainWindow::onClearRoute);

    routeMenu->addSeparator();

    QAction* executeAction = routeMenu->addAction("&Execute Route");
    executeAction->setShortcut(QKeySequence("Ctrl+X"));
    connect(executeAction, &QAction::triggered, this, &MainWindow::onExecuteRoute);

    routeMenu->addSeparator();

    QAction* editVNAVAction = routeMenu->addAction("Edit &VNAV Constraints...");
    editVNAVAction->setShortcut(QKeySequence("Ctrl+V"));
    connect(editVNAVAction, &QAction::triggered, this, &MainWindow::onEditVNAVConstraints);

    QAction* applySTARAction = routeMenu->addAction("Apply Sample STAR &Constraints");
    connect(applySTARAction, &QAction::triggered, this, [this]() {
        applyDefaultSTARConstraints();
    });

    QMenu* helpMenu = menuBar->addMenu("&Help");

    QAction* aboutAction = helpMenu->addAction("&About FMC...");
    aboutAction->setShortcut(QKeySequence("F1"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupConnections() {
    connect(m_cduWidget, &CDUWidget::flightPlanChanged,
            this, &MainWindow::onFlightPlanChanged);
    connect(m_cduWidget, &CDUWidget::routeExecuted,
            this, &MainWindow::onRouteExecuted);
    connect(m_vnavWidget, &VNAVProfileWidget::waypointClicked,
            this, &MainWindow::onVNAVWaypointClicked);
}

void MainWindow::loadDefaultDatabase() {
    QString defaultPath = QStringLiteral("%1/%2")
                         .arg(QCoreApplication::applicationDirPath())
                         .arg(DEFAULT_DATA_PATH);

    QFile file(defaultPath);
    if (!file.exists()) {
        defaultPath = DEFAULT_DATA_PATH;
    }

    QFileInfo fi(defaultPath);
    if (fi.exists()) {
        m_database = m_parser.parseFile(defaultPath.toStdString());
        if (m_parser.getStatistics().errorRecords == 0 ||
            m_database.waypoints.size() > 0) {
            m_databaseLoaded = true;
            m_cduWidget->setDatabase(m_database);
            updateStatusBar();
            return;
        }
    }

    QString sampleData = generateSampleARINC424Data();
    m_database = m_parser.parseString(sampleData.toStdString());
    m_databaseLoaded = true;
    m_cduWidget->setDatabase(m_database);
    updateStatusBar();
}

void MainWindow::onLoadDatabase() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load ARINC 424 Database",
        "",
        "ARINC 424 Files (*.txt *.dat *.424);;All Files (*.*)");

    if (fileName.isEmpty()) return;

    m_database = m_parser.parseFile(fileName.toStdString());
    const auto& stats = m_parser.getStatistics();

    if (stats.errorRecords > 0 && m_database.waypoints.empty()) {
        QMessageBox::warning(this, "Parse Error",
                             QString("Failed to parse ARINC 424 file.\n\n"
                                     "Errors: %1 records\n"
                                     "Check the file format.")
                                 .arg(stats.errorRecords));
        return;
    }

    m_databaseLoaded = true;
    m_cduWidget->setDatabase(m_database);
    updateStatusBar();

    QString message = QString("ARINC 424 Database Loaded Successfully\n\n"
                              "Total Records: %1\n"
                              "Waypoints:     %2\n"
                              "Navaids:       %3\n"
                              "Airways:       %4\n"
                              "Airports:      %5\n"
                              "Skipped:       %6\n"
                              "Errors:        %7")
                          .arg(stats.totalRecords)
                          .arg(stats.waypointRecords)
                          .arg(stats.navaidRecords)
                          .arg(stats.airwayRecords)
                          .arg(stats.airportRecords)
                          .arg(stats.skippedRecords)
                          .arg(stats.errorRecords);

    QMessageBox::information(this, "Database Loaded", message);
}

void MainWindow::onLoadDefaultRoute() {
    if (!m_databaseLoaded) {
        QMessageBox::warning(this, "Database Not Loaded",
                             "Please load an ARINC 424 database first.");
        return;
    }
    m_cduWidget->loadDefaultRoute();
}

void MainWindow::onClearRoute() {
    m_cduWidget->clearRoute();
    m_lnavTable->clearFlightPlan();
    m_vnavWidget->clearProfile();
    m_altitudeConstraints.clear();
    m_vnavProfile = nav::VNAVProfile();
    m_routeExecuted = false;
    m_simulatedLegIndex = -1;
    if (m_flightSimTimer) m_flightSimTimer->stop();
    updateStatusBar();
    if (m_statusVNAVLabel) {
        m_statusVNAVLabel->setText("VNAV: CLEARED");
        m_statusVNAVLabel->setStyleSheet("color: rgb(200, 200, 200); padding: 0 10px;");
    }
}

void MainWindow::onExecuteRoute() {
    m_cduWidget->executeRoute();
}

void MainWindow::onExportFlightPlan() {
    if (m_currentPlan.legs.empty()) {
        QMessageBox::warning(this, "No Flight Plan",
                             "Please create and execute a flight plan first.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Flight Plan",
        QString("FlightPlan_%1.txt")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmm")),
        "Text Files (*.txt);;All Files (*.*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Failed",
                              "Failed to open file for writing.");
        return;
    }

    QTextStream out(&file);
    out << "===========================================================\n";
    out << "   FLIGHT MANAGEMENT COMPUTER - FLIGHT PLAN\n";
    out << "===========================================================\n\n";
    out << "Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";
    out << QString("Flight:   %1\n").arg(QString::fromStdString(m_currentPlan.flightNumber));
    out << QString("Origin:   %1\n").arg(QString::fromStdString(m_currentPlan.origin));
    out << QString("Dest:     %1\n").arg(QString::fromStdString(m_currentPlan.destination));
    out << QString("Alternate: %1\n\n").arg(QString::fromStdString(m_currentPlan.alternate));
    out << "===========================================================\n";
    out << QString("Total Distance: %1 NM / %2 KM\n")
               .arg(m_currentPlan.totalDistanceNm, 0, 'f', 2)
               .arg(m_currentPlan.totalDistanceKm, 0, 'f', 2);
    out << QString("Est. Total Time: %1 minutes\n\n")
               .arg(m_currentPlan.estimatedTotalTime, 0, 'f', 1);
    out << "===========================================================\n";
    out << "SEQ  WAYPOINT  AIRWAY  LATITUDE     LONGITUDE     DIST   CUM    TRK\n";
    out << "===========================================================\n";

    for (size_t i = 0; i < m_currentPlan.legs.size(); ++i) {
        const auto& leg = m_currentPlan.legs[i];
        out << QString("%1  %2  %3  %4   %5  %6  %7  %8\n")
                   .arg(leg.sequenceNumber, 3)
                   .arg(QString::fromStdString(leg.waypointIdentifier), -8)
                   .arg(QString::fromStdString(leg.airway), -6)
                   .arg(m_lnavTable->formatLatitude(leg.position.latitude))
                   .arg(m_lnavTable->formatLongitude(leg.position.longitude))
                   .arg(leg.distanceFromOrigin, 5, 'f', 1)
                   .arg(leg.cumulativeDistance, 5, 'f', 1)
                   .arg(static_cast<int>(leg.trueTrack), 3);
    }

    out << "===========================================================\n";
    file.close();

    QMessageBox::information(this, "Export Complete",
                         QString("Flight plan exported to:\n%1").arg(fileName));
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About FMC Simulator",
                   R"(<h2>Flight Management Computer Simulator</h2>
                    <p><b>Version 1.0</b></p>
                    <p>A highly realistic flight management computer simulation for large passenger aircraft.</p>
                    <h3>Features:</h3>
                    <ul>
                    <li>ARINC 424 fixed-width string parser</li>
                    <li>Great Circle distance & bearing calculation</li>
                    <li>CDU (Control Display Unit) interface</li>
                    <li>LNAV horizontal flight planning</li>
                    <li>State machine for route planning</li>
                    </ul>
                    <p><b>Technical Stack:</b> C++17, Qt 6.8</p>
                    <p><b>© 2026 Aviation Simulation Systems</b></p>)");
}

void MainWindow::onFlightPlanChanged(const nav::FlightPlan& plan) {
    m_currentPlan = plan;
    m_lnavTable->setFlightPlan(plan);
    updateStatusBar();
}

void MainWindow::onRouteExecuted(const nav::FlightPlan& plan) {
    m_currentPlan = plan;
    m_routeExecuted = true;
    m_simulatedLegIndex = 0;
    m_lnavTable->setFlightPlan(plan);

    runVNAVSolver();

    if (!m_flightSimTimer->isActive()) {
        m_flightSimTimer->start();
    }

    updateStatusBar();
}

void MainWindow::runVNAVSolver() {
    if (m_currentPlan.legs.size() < 2) {
        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText("VNAV: INSUFFICIENT WAYPOINTS");
            m_statusVNAVLabel->setStyleSheet("color: rgb(255, 180, 0); padding: 0 10px;");
        }
        return;
    }

    if (m_altitudeConstraints.size() < m_currentPlan.legs.size()) {
        m_altitudeConstraints.resize(m_currentPlan.legs.size());
    }

    try {
        m_vnavSolver.setAircraftPerformance(nav::AircraftPerformance::boeing737());
        m_vnavSolver.setFlightPlan(m_currentPlan);
        m_vnavSolver.setAltitudeConstraints(m_altitudeConstraints);
        m_vnavSolver.setCruiseAltitude(36000.0);

        m_vnavProfile = m_vnavSolver.solve();

        m_vnavWidget->setVNAVProfile(m_vnavProfile);

        QString statusStr;
        QString statusColor;
        switch (m_vnavProfile.status) {
            case nav::VNAVSolverStatus::Success:
                statusStr = QString("VNAV: OK | CRZ %1FT | TOD %2NM")
                                .arg(m_vnavProfile.cruiseAltitudeFt, 0, 'f', 0)
                                .arg(m_vnavProfile.topOfDescentDistanceNm, 0, 'f', 0);
                statusColor = "rgb(0, 255, 150);";
                break;
            case nav::VNAVSolverStatus::SuccessWithWarnings:
                statusStr = QString("VNAV: WARNING | %1 relaxed")
                                .arg(m_vnavProfile.numConstraintsRelaxed);
                if (m_vnavProfile.gradientTruncationApplied)
                    statusStr += " | GRAD TRUNC";
                statusColor = "rgb(255, 180, 0);";
                break;
            case nav::VNAVSolverStatus::ConstraintInfeasible:
                statusStr = QString("VNAV: INFEASIBLE | %1 violations")
                                .arg(m_vnavProfile.numConstraintsRelaxed);
                statusColor = "rgb(255, 80, 80);";
                break;
            default:
                statusStr = "VNAV: ERROR";
                statusColor = "rgb(255, 80, 80);";
                break;
        }

        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText(statusStr);
            m_statusVNAVLabel->setStyleSheet(QString("color: %1 padding: 0 10px;").arg(statusColor));
        }

    } catch (const std::exception& e) {
        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText(QString("VNAV: CRASH AVOIDED - %1")
                .arg(QString::fromLocal8Bit(e.what()).left(40)));
            m_statusVNAVLabel->setStyleSheet("color: rgb(255, 80, 80); padding: 0 10px;");
        }
    } catch (...) {
        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText("VNAV: CRASH AVOIDED - UNKNOWN");
            m_statusVNAVLabel->setStyleSheet("color: rgb(255, 80, 80); padding: 0 10px;");
        }
    }
}

nav::AltitudeConstraint MainWindow::getWaypointConstraint(size_t legIndex) const {
    if (legIndex < m_altitudeConstraints.size()) {
        return m_altitudeConstraints[legIndex];
    }
    return nav::AltitudeConstraint();
}

void MainWindow::setWaypointConstraint(size_t legIndex, const nav::AltitudeConstraint& c) {
    if (legIndex >= m_altitudeConstraints.size()) {
        m_altitudeConstraints.resize(legIndex + 1);
    }
    m_altitudeConstraints[legIndex] = c;
}

void MainWindow::applyDefaultSTARConstraints() {
    if (m_currentPlan.legs.empty()) {
        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText("VNAV: NO ROUTE - CREATE FIRST");
            m_statusVNAVLabel->setStyleSheet("color: rgb(255, 180, 0); padding: 0 10px;");
        }
        return;
    }

    size_t n = m_currentPlan.legs.size();
    m_altitudeConstraints.assign(n, nav::AltitudeConstraint());

    if (n >= 1) {
        m_altitudeConstraints[0] = nav::AltitudeConstraint::makeAt(200.0, "DEPARTURE RWY");
    }
    if (n >= 2) {
        m_altitudeConstraints[n-1] = nav::AltitudeConstraint::makeAt(100.0, "DESTINATION RWY");
    }

    if (n >= 6) {
        size_t approach1 = n - 2;
        size_t approach2 = n - 3;
        size_t terminal = n - 4;

        if (approach2 < n) {
            m_altitudeConstraints[approach2] = nav::AltitudeConstraint::makeAt(
                6000.0, "STAR IAF - 6000FT");
        }
        if (approach1 < n) {
            m_altitudeConstraints[approach1] = nav::AltitudeConstraint::makeBelow(
                10000.0, "ATC 10000FT LIMIT");
        }
        if (terminal < n) {
            m_altitudeConstraints[terminal] = nav::AltitudeConstraint::makeBelow(
                12000.0, "TERMINAL 12000FT");
        }
    }

    if (m_routeExecuted) {
        runVNAVSolver();
    } else {
        if (m_statusVNAVLabel) {
            m_statusVNAVLabel->setText("VNAV: CONSTRAINTS SET - EXECUTE ROUTE");
            m_statusVNAVLabel->setStyleSheet("color: rgb(100, 200, 255); padding: 0 10px;");
        }
    }
}

void MainWindow::onVNAVWaypointClicked(size_t /*legIndex*/) {
    onEditVNAVConstraints();
}

void MainWindow::onEditVNAVConstraints() {
    if (m_currentPlan.legs.empty()) {
        QMessageBox::warning(this, "No Route",
                             "Please create and execute a route first before editing constraints.");
        return;
    }

    if (m_altitudeConstraints.size() < m_currentPlan.legs.size()) {
        m_altitudeConstraints.resize(m_currentPlan.legs.size());
    }

    QString summary;
    QTextStream ss(&summary);
    ss << "<h3 style='color:#00FFFF;'>✈ VNAV Altitude Constraint Editor</h3>"
       << "<p style='color:#7FD4FF;'>Current aircraft: <b>Boeing 737</b></p>"
       << "<p style='color:#FFD000;'>Max descent gradient: 1500ft/NM | Max climb gradient: 600ft/NM</p>"
       << "<hr>"
       << "<table border='1' cellpadding='4' cellspacing='0' "
       << "style='font-family:Consolas,monospace; font-size:10pt; border-collapse:collapse;'>"
       << "<tr style='background:#102040; color:#00FFFF;'>"
       << "<th>SEQ</th><th>WAYPOINT</th><th>DIST(NM)</th>"
       << "<th>CONSTRAINT</th><th>SOURCE</th></tr>";

    for (size_t i = 0; i < m_currentPlan.legs.size(); ++i) {
        const auto& leg = m_currentPlan.legs[i];
        const auto& c = m_altitudeConstraints[i];

        QString constraintTxt;
        QString color = leg.altitude.has_value() ? "#7FFF7F" : "#FFFFFF";
        switch (c.type) {
            case nav::AltitudeConstraintType::None:
                constraintTxt = "<span style='color:#888888;'>NONE</span>";
                break;
            case nav::AltitudeConstraintType::At:
                constraintTxt = QString("@ <b>%1ft</b>").arg(c.altitudeAtFt, 0, 'f', 0);
                color = "#FFD000";
                break;
            case nav::AltitudeConstraintType::AtOrAbove:
                constraintTxt = QString("A+ ≥ <b>%1ft</b>").arg(c.altitudeAboveFt, 0, 'f', 0);
                color = "#00FF80";
                break;
            case nav::AltitudeConstraintType::AtOrBelow:
                constraintTxt = QString("B- ≤ <b>%1ft</b>").arg(c.altitudeBelowFt, 0, 'f', 0);
                color = "#FFA000";
                break;
            case nav::AltitudeConstraintType::Between:
                constraintTxt = QString("[%1, %2]ft")
                    .arg(c.altitudeAboveFt, 0, 'f', 0)
                    .arg(c.altitudeBelowFt, 0, 'f', 0);
                color = "#00FFFF";
                break;
        }

        QString src = c.source.empty() ? "-" : QString::fromStdString(c.source);

        ss << QString("<tr style='color:%1;'><td align='right'>%2</td>"
                      "<td><b>%3</b></td><td align='right'>%4</td>"
                      "<td>%5</td><td>%6</td></tr>")
                  .arg(color)
                  .arg(i + 1)
                  .arg(QString::fromStdString(leg.waypointIdentifier))
                  .arg(leg.cumulativeDistance, 0, 'f', 1)
                  .arg(constraintTxt)
                  .arg(src);
    }
    ss << "</table><br>";

    if (!m_vnavProfile.warnings.empty()) {
        ss << "<h4 style='color:#FFA000;'>⚠ Last VNAV Solver Warnings:</h4>"
           << "<ul style='color:#FFD080; font-size:9pt;'>";
        size_t showN = std::min(m_vnavProfile.warnings.size(), size_t(5));
        for (size_t i = 0; i < showN; ++i) {
            ss << "<li>" << QString::fromStdString(m_vnavProfile.warnings[i]) << "</li>";
        }
        if (m_vnavProfile.warnings.size() > showN)
            ss << "<li>...and " << (m_vnavProfile.warnings.size() - showN) << " more</li>";
        ss << "</ul>";
    }

    ss << "<hr><p style='color:#7FD4FF;'>"
       << "For demo purposes, click buttons below to apply constraint patterns.<br>"
       << "The solver uses <b>Dynamic Programming with gradient truncation</b> - "
       << "it CANNOT crash or recurse infinitely.</p>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("VNAV Constraint Editor");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(summary);

    QPushButton* applySTAR = msgBox.addButton("Apply STAR Pattern", QMessageBox::ActionRole);
    QPushButton* applyExtreme = msgBox.addButton("Apply IMPOSSIBLE Pattern (Test)", QMessageBox::ActionRole);
    QPushButton* clearAllBtn = msgBox.addButton("Clear All Constraints", QMessageBox::ActionRole);
    QPushButton* okBtn = msgBox.addButton(QMessageBox::Ok);
    msgBox.setDefaultButton(okBtn);

    Q_UNUSED(okBtn);

    msgBox.exec();

    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == applySTAR) {
        applyDefaultSTARConstraints();
        onEditVNAVConstraints();
    } else if (clicked == applyExtreme) {
        size_t n = m_currentPlan.legs.size();
        m_altitudeConstraints.assign(n, nav::AltitudeConstraint());
        if (n >= 1) m_altitudeConstraints[0] = nav::AltitudeConstraint::makeAt(150.0, "DEPT");
        if (n >= 5) {
            for (size_t i = 1; i < n - 1; ++i) {
                if (i < n / 2) {
                    m_altitudeConstraints[i] = nav::AltitudeConstraint::makeAt(
                        40000.0 - i * 5000.0, "TEST: DEEP DESCENT");
                } else {
                    m_altitudeConstraints[i] = nav::AltitudeConstraint::makeAt(
                        1500.0 + i * 2000.0, "TEST: IMPOSSIBLE CLIMB");
                }
            }
        }
        if (n >= 2) m_altitudeConstraints[n-1] = nav::AltitudeConstraint::makeAt(200.0, "DEST");
        if (m_routeExecuted) runVNAVSolver();
        onEditVNAVConstraints();
    } else if (clicked == clearAllBtn) {
        m_altitudeConstraints.assign(m_currentPlan.legs.size(), nav::AltitudeConstraint());
        if (m_routeExecuted) runVNAVSolver();
        onEditVNAVConstraints();
    }
}

void MainWindow::updateStatusBar() {
    if (m_databaseLoaded) {
        m_statusDatabaseLabel->setText(QString("DB: ARINC 424 ACTIVE"));
        m_statusDatabaseLabel->setStyleSheet("color: rgb(0, 255, 150); padding: 0 10px;");
        m_statusWaypointsLabel->setText(QString("WPT: %1 / APT: %2")
                                    .arg(m_database.waypoints.size())
                                    .arg(m_database.airports.size()));
    } else {
        m_statusDatabaseLabel->setText("DB: NOT LOADED");
        m_statusDatabaseLabel->setStyleSheet("color: rgb(255, 100, 100); padding: 0 10px;");
    }

    if (m_currentPlan.legs.empty()) {
        m_statusDistanceLabel->setText("DIST: -- NM");
        m_statusFuelLabel->setText("FUEL: -- KG");
        m_statusTimeLabel->setText("TIME: --:--");
    } else {
        m_statusDistanceLabel->setText(QString("DIST: %1 NM / %2 KM")
                                    .arg(m_currentPlan.totalDistanceNm, 0, 'f', 0)
                                    .arg(m_currentPlan.totalDistanceKm, 0, 'f', 0));

        double fuelKg = m_currentPlan.totalDistanceNm * 8.5;
        m_statusFuelLabel->setText(QString("FUEL: %1 KG")
                                    .arg(static_cast<quint64>(fuelKg)));

        int totalMin = static_cast<int>(m_currentPlan.estimatedTotalTime);
        int hours = totalMin / 60;
        int mins = totalMin % 60;
        m_statusTimeLabel->setText(QString("TIME: %1:%2")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(mins, 2, 10, QChar('0')));
    }
}

void MainWindow::simulateFlightProgress() {
    if (m_routeExecuted && m_simulatedLegIndex >= 0 &&
        m_simulatedLegIndex < static_cast<int>(m_currentPlan.legs.size())) {
        m_lnavTable->updateActiveLeg(m_simulatedLegIndex);
        m_simulatedLegIndex++;

        if (m_simulatedLegIndex >= static_cast<int>(m_currentPlan.legs.size())) {
            m_flightSimTimer->stop();
        }
    }
}

QString MainWindow::generateSampleARINC424Data() const {
    QString sampleData;
    QTextStream out(&sampleData);

    out << "SADPA  BEIJING CAPITAL                       ZBAA  N40045200E116345400  116    00000E065000                                                            BEIJING CAPITAL INTL               \n";
    out << "SADPA  SHANGHAI PUDONG                      ZSPD  N31084200E121481800   13    00000E06000                                                            SHANGHAI PUDONG INTL                 \n";
    out << "SADPA  GUANGZHOU BAIYUN                     ZGGG  N23233300E113180700   35    00000E065000                                                            GUANGZHOU BAIYUN INTL                \n";
    out << "SADPA  CHENGDU SHUANGLIU                   ZUUU  N30373900E103564900  1624    00000E060000                                                            CHENGDU SHUANGLIU INTL               \n";
    out << "SADPA  KUNMING CHANGSHUI                   ZPPP  N25062600E102554800  6890    00000E060000                                                            KUNMING CHANGSHUI INTL              \n";
    out << "SADPA  XIANYANG XIAN                       ZLXY  N34263600E108452400  1560    00000E060000                                                            XIAN XIANYANG INTL                   \n";
    out << "SADPA  HANGZHOU XIAOSHAN                   ZSHC  N30140400E120260400   23    00000E060000                                                            HANGZHOU XIAOSHAN INTL               \n";
    out << "EAASP  W58                              ZBPEAN40120000E117000000                                                                                            \n";
    out << "EAASP  PIKAS                            ZBPEAN40300000E118000000                                                                                            \n";
    out << "EAASP  LADIX                            ZBPEAN41000000E119000000                                                                                            \n";
    out << "EAASP  TOBAK                            ZBPEAN41300000E12000000                                                                                            \n";
    out << "EAASP  DALIM                            ZBPEAN32000000E118300000                                                                                            \n";
    out << "EAASP  GUMOL                            ZBPEAN31000000E119300000                                                                                            \n";
    out << "EAASP  BESRA                            ZBPEAN30300000E120300000                                                                                            \n";
    out << "EAASP  PIMOS                            ZBPEAN31300000E121000000                                                                                            \n";
    out << "ERASP  A1    W58     ZBPEA                                                                                              090080               PIKAS     \n";
    out << "ERASP  A1    PIKAS   ZBPEA                                                                                              090120               LADIX     \n";
    out << "ERASP  A1    LADIX   ZBPEA                                                                                              090150               TOBAK     \n";
    out << "ERASP  A1    TOBAK   ZBPEA                                                                                              150100               PIMOS     \n";
    out << "ERASP  A1    PIMOS   ZBPEA                                                                                              150080               BESRA     \n";
    out << "ERASP  A1    BESRA   ZBPEA                                                                                              150060               GUMOL     \n";
    out << "ERASP  A1    GUMOL   ZBPEA                                                                                              150050               DALIM     \n";
    out << "DASP   PEK   VOR DMEZBPEAN40045200E116345400  11611470 180E06500                                                            \n";
    out << "DASP   PVG   VOR DMEZSPDAN31084200E121481800   1311470 120E06000                                                            \n";
    out << "DASP   CAN   VOR DMEZGGAN23233300E113180700   3511430 200E06500                                                            \n";
    out << "DASP   CTU   VOR DMEZUUUN30373900E103564900 162411570 180E06000                                                            \n";

    return sampleData;
}

}
}
