#include "cdu_widget.h"
#include <QFontDatabase>
#include <QPalette>
#include <QMessageBox>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace fmc {
namespace gui {

const QColor CDUWidget::CDU_BG_COLOR(30, 35, 45);
const QColor CDUWidget::CDU_DISPLAY_BG_COLOR(5, 10, 20);
const QColor CDUWidget::CDU_TEXT_COLOR(220, 220, 220);
const QColor CDUWidget::CDU_AMBER_COLOR(255, 180, 0);
const QColor CDUWidget::CDU_GREEN_COLOR(0, 255, 120);
const QColor CDUWidget::CDU_CYAN_COLOR(0, 200, 255);
const QColor CDUWidget::CDU_MAGENTA_COLOR(255, 80, 255);
const QColor CDUWidget::CDU_WHITE_COLOR(255, 255, 255);
const QColor CDUWidget::CDU_RED_COLOR(255, 60, 60);

CDUWidget::CDUWidget(QWidget* parent)
    : QWidget(parent),
      m_routePlanner(nullptr),
      m_currentPage(PAGE_ROUTE),
      m_scrollOffset(0),
      m_blinkState(false) {
    m_routePlanner = new nav::RoutePlanner();
    setupUI();

    m_blinkTimer = new QTimer(this);
    connect(m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkState = !m_blinkState;
        updateDisplay();
    });
    m_blinkTimer->start(500);
}

void CDUWidget::setupUI() {
    setStyleSheet(QString(R"(
        QWidget {
            background-color: rgb(%1, %2, %3);
        }
    )").arg(CDU_BG_COLOR.red()).arg(CDU_BG_COLOR.green()).arg(CDU_BG_COLOR.blue()));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    setupDisplayArea();
    mainLayout->addWidget(m_displayFrame, 3);

    setupKeypad();
    mainLayout->addWidget(m_keypadFrame, 5);

    setLayout(mainLayout);
}

void CDUWidget::setupDisplayArea() {
    m_displayFrame = new QFrame();
    m_displayFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_displayFrame->setStyleSheet(QString(R"(
        QFrame {
            background-color: rgb(%1, %2, %3);
            border: 3px solid rgb(60, 70, 90);
            border-radius: 6px;
        }
    )").arg(CDU_DISPLAY_BG_COLOR.red()).arg(CDU_DISPLAY_BG_COLOR.green()).arg(CDU_DISPLAY_BG_COLOR.blue()));

    m_displayLayout = new QVBoxLayout(m_displayFrame);
    m_displayLayout->setContentsMargins(12, 12, 12, 12);
    m_displayLayout->setSpacing(4);

    m_titleLabel = new QLabel("FMC - FLIGHT MANAGEMENT COMPUTER");
    m_titleLabel->setStyleSheet(QString(R"(
        QLabel {
            color: rgb(%1, %2, %3);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 12px;
            font-weight: bold;
            background-color: transparent;
        }
    )").arg(CDU_AMBER_COLOR.red()).arg(CDU_AMBER_COLOR.green()).arg(CDU_AMBER_COLOR.blue()));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_displayLayout->addWidget(m_titleLabel);

    m_statusLabel = new QLabel("STATUS: ACTIVE");
    m_statusLabel->setStyleSheet(QString(R"(
        QLabel {
            color: rgb(%1, %2, %3);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 10px;
            background-color: transparent;
        }
    )").arg(CDU_GREEN_COLOR.red()).arg(CDU_GREEN_COLOR.green()).arg(CDU_GREEN_COLOR.blue()));
    m_statusLabel->setAlignment(Qt::AlignRight);
    m_displayLayout->addWidget(m_statusLabel);

    m_scratchpadList = new QListWidget();
    m_scratchpadList->setStyleSheet(QString(R"(
        QListWidget {
            background-color: rgba(0, 0, 0, 50);
            border: 1px solid rgb(40, 60, 80);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 11px;
            color: rgb(%1, %2, %3);
            padding: 4px;
        }
        QListWidget::item {
            padding: 2px;
        }
    )").arg(CDU_TEXT_COLOR.red()).arg(CDU_TEXT_COLOR.green()).arg(CDU_TEXT_COLOR.blue()));
    m_scratchpadList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scratchpadList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scratchpadList->setFixedHeight(100);
    m_displayLayout->addWidget(m_scratchpadList);

    m_inputLineLabel = new QLabel("INPUT>");
    m_inputLineLabel->setStyleSheet(QString(R"(
        QLabel {
            color: rgb(%1, %2, %3);
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 12px;
            font-weight: bold;
            background-color: transparent;
            padding-top: 6px;
            border-top: 1px solid rgb(60, 80, 100);
        }
    )").arg(CDU_CYAN_COLOR.red()).arg(CDU_CYAN_COLOR.green()).arg(CDU_CYAN_COLOR.blue()));
    m_displayLayout->addWidget(m_inputLineLabel);

    m_inputLineEdit = new QLineEdit();
    m_inputLineEdit->setReadOnly(true);
    m_inputLineEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: rgba(0, 20, 40, 200);
            border: 2px solid rgb(0, 120, 180);
            border-radius: 4px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 14px;
            color: rgb(%1, %2, %3);
            padding: 6px;
            selection-background-color: rgb(255, 200, 0);
        }
    )").arg(CDU_GREEN_COLOR.red()).arg(CDU_GREEN_COLOR.green()).arg(CDU_GREEN_COLOR.blue()));
    m_inputLineEdit->setPlaceholderText("ENTER DATA...");
    m_displayLayout->addWidget(m_inputLineEdit);

    m_displayFrame->setLayout(m_displayLayout);
}

void CDUWidget::setupKeypad() {
    m_keypadFrame = new QFrame();
    m_keypadFrame->setStyleSheet(R"(
        QFrame {
            background-color: rgb(40, 45, 55);
            border: 2px solid rgb(60, 70, 80);
            border-radius: 8px;
        }
    )");

    m_keypadLayout = new QGridLayout(m_keypadFrame);
    m_keypadLayout->setContentsMargins(12, 12, 12, 12);
    m_keypadLayout->setSpacing(6);

    createFunctionKey("INIT", nav::CDUKey::Key_Init, 0, 0);
    createFunctionKey("ROUTE", nav::CDUKey::Key_Route, 0, 1);
    createFunctionKey("LEGS", nav::CDUKey::Key_Legs, 0, 2);
    createFunctionKey("PROG", nav::CDUKey::Key_Prog, 0, 3);
    createFunctionKey("NAV", nav::CDUKey::Key_Nav, 0, 4);
    createFunctionKey("PERF", nav::CDUKey::Key_Perf, 0, 5);

    createKeyButton("1", nav::CDUKey::Key_1, 1, 0);
    createKeyButton("2", nav::CDUKey::Key_2, 1, 1);
    createKeyButton("3", nav::CDUKey::Key_3, 1, 2);
    createKeyButton("4", nav::CDUKey::Key_4, 1, 3);
    createKeyButton("5", nav::CDUKey::Key_5, 1, 4);
    createKeyButton("6", nav::CDUKey::Key_6, 1, 5);
    createKeyButton("7", nav::CDUKey::Key_7, 2, 0);
    createKeyButton("8", nav::CDUKey::Key_8, 2, 1);
    createKeyButton("9", nav::CDUKey::Key_9, 2, 2);
    createKeyButton(".", nav::CDUKey::Key_Dot, 2, 3);
    createKeyButton("0", nav::CDUKey::Key_0, 2, 4);
    createKeyButton("/", nav::CDUKey::Key_Slash, 2, 5);

    createKeyButton("A", nav::CDUKey::Key_A, 3, 0);
    createKeyButton("B", nav::CDUKey::Key_B, 3, 1);
    createKeyButton("C", nav::CDUKey::Key_C, 3, 2);
    createKeyButton("D", nav::CDUKey::Key_D, 3, 3);
    createKeyButton("E", nav::CDUKey::Key_E, 3, 4);
    createKeyButton("F", nav::CDUKey::Key_F, 3, 5);
    createKeyButton("G", nav::CDUKey::Key_G, 4, 0);
    createKeyButton("H", nav::CDUKey::Key_H, 4, 1);
    createKeyButton("I", nav::CDUKey::Key_I, 4, 2);
    createKeyButton("J", nav::CDUKey::Key_J, 4, 3);
    createKeyButton("K", nav::CDUKey::Key_K, 4, 4);
    createKeyButton("L", nav::CDUKey::Key_L, 4, 5);
    createKeyButton("M", nav::CDUKey::Key_M, 5, 0);
    createKeyButton("N", nav::CDUKey::Key_N, 5, 1);
    createKeyButton("O", nav::CDUKey::Key_O, 5, 2);
    createKeyButton("P", nav::CDUKey::Key_P, 5, 3);
    createKeyButton("Q", nav::CDUKey::Key_Q, 5, 4);
    createKeyButton("R", nav::CDUKey::Key_R, 5, 5);
    createKeyButton("S", nav::CDUKey::Key_S, 6, 0);
    createKeyButton("T", nav::CDUKey::Key_T, 6, 1);
    createKeyButton("U", nav::CDUKey::Key_U, 6, 2);
    createKeyButton("V", nav::CDUKey::Key_V, 6, 3);
    createKeyButton("W", nav::CDUKey::Key_W, 6, 4);
    createKeyButton("X", nav::CDUKey::Key_X, 6, 5);
    createKeyButton("Y", nav::CDUKey::Key_Y, 7, 0);
    createKeyButton("Z", nav::CDUKey::Key_Z, 7, 1);
    createKeyButton("+", nav::CDUKey::Key_Plus, 7, 2);
    createKeyButton("-", nav::CDUKey::Key_Minus, 7, 3);

    createKeyButton("ORIGIN", nav::CDUKey::Key_Origin, 1, 6);
    createKeyButton("DEST", nav::CDUKey::Key_Dest, 2, 6);
    createKeyButton("ALTN", nav::CDUKey::Key_Alt, 3, 6);

    createKeyButton("CLR", nav::CDUKey::Key_Clear, 4, 6);
    createKeyButton("DEL", nav::CDUKey::Key_Delete, 5, 6);
    createKeyButton("CANCEL", nav::CDUKey::Key_Cancel, 6, 6);
    createKeyButton("EXEC", nav::CDUKey::Key_Exec, 7, 6);

    m_keypadFrame->setLayout(m_keypadLayout);
}

void CDUWidget::createKeyButton(const QString& label, nav::CDUKey key, int row, int col,
                                int rowSpan, int colSpan) {
    QPushButton* btn = new QPushButton(label);
    btn->setMinimumHeight(38);
    btn->setCursor(Qt::PointingHandCursor);

    QString btnStyle = R"(
        QPushButton {
            background-color: rgb(50, 60, 75);
            color: rgb(220, 220, 220);
            border: 2px solid rgb(80, 90, 110);
            border-radius: 5px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 11px;
            font-weight: bold;
            padding: 4px 8px;
        }
        QPushButton:hover {
            background-color: rgb(70, 80, 95);
            border-color: rgb(120, 140, 170);
        }
        QPushButton:pressed {
            background-color: rgb(30, 40, 55);
            border-color: rgb(255, 200, 0);
            color: rgb(255, 200, 0);
        }
    )";

    if (key == nav::CDUKey::Key_Exec) {
        btnStyle = R"(
            QPushButton {
                background-color: rgb(255, 180, 0);
                color: rgb(0, 0, 0);
                border: 2px solid rgb(255, 220, 80);
                border-radius: 5px;
                font-family: 'Consolas', 'Courier New', monospace;
                font-size: 11px;
                font-weight: bold;
                padding: 4px 8px;
            }
            QPushButton:hover {
                background-color: rgb(255, 200, 40);
            }
            QPushButton:pressed {
                background-color: rgb(220, 150, 0);
            }
        )";
    } else if (key == nav::CDUKey::Key_Clear || key == nav::CDUKey::Key_Delete ||
               key == nav::CDUKey::Key_Cancel) {
        btnStyle = R"(
            QPushButton {
                background-color: rgb(120, 40, 40);
                color: rgb(255, 200, 200);
                border: 2px solid rgb(160, 60, 60);
                border-radius: 5px;
                font-family: 'Consolas', 'Courier New', monospace;
                font-size: 11px;
                font-weight: bold;
                padding: 4px 8px;
            }
            QPushButton:hover {
                background-color: rgb(150, 50, 50);
            }
            QPushButton:pressed {
                background-color: rgb(100, 30, 30);
            }
        )";
    }

    btn->setStyleSheet(btnStyle);
    connect(btn, &QPushButton::clicked, this, [this, key]() {
        handleKeyPress(key);
    });

    m_keypadLayout->addWidget(btn, row, col, rowSpan, colSpan);
}

void CDUWidget::createFunctionKey(const QString& label, nav::CDUKey key, int row, int col) {
    QPushButton* btn = new QPushButton(label);
    btn->setMinimumHeight(32);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(R"(
        QPushButton {
            background-color: rgb(30, 50, 80);
            color: rgb(0, 200, 255);
            border: 2px solid rgb(50, 100, 150);
            border-radius: 5px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 10px;
            font-weight: bold;
            padding: 4px 8px;
        }
        QPushButton:hover {
            background-color: rgb(40, 70, 110);
        }
        QPushButton:pressed {
            background-color: rgb(20, 40, 70);
            border-color: rgb(0, 200, 255);
        }
    )");
    connect(btn, &QPushButton::clicked, this, [this, key]() {
        handleKeyPress(key);
    });
    m_keypadLayout->addWidget(btn, row, col);
}

void CDUWidget::handleKeyPress(nav::CDUKey key) {
    if (key == nav::CDUKey::Key_Route) {
        m_currentPage = PAGE_ROUTE;
        m_stateMachine.setState(nav::CDUInputState::EnteringRoute);
        appendScratchpadMessage("ROUTE PAGE SELECTED");
    } else if (key == nav::CDUKey::Key_Legs) {
        m_currentPage = PAGE_LEGS;
        appendScratchpadMessage("LEGS PAGE SELECTED");
    } else if (key == nav::CDUKey::Key_Init) {
        m_currentPage = PAGE_INIT;
        appendScratchpadMessage("INIT PAGE SELECTED");
    } else if (key == nav::CDUKey::Key_Perf) {
        m_currentPage = PAGE_PERF;
        appendScratchpadMessage("PERF PAGE SELECTED");
    } else if (key == nav::CDUKey::Key_Nav) {
        m_currentPage = PAGE_NAV;
        appendScratchpadMessage("NAV PAGE SELECTED");
    } else if (key == nav::CDUKey::Key_Origin) {
        m_stateMachine.setState(nav::CDUInputState::EnteringOrigin);
        appendScratchpadMessage("ENTER ORIGIN AIRPORT:");
    } else if (key == nav::CDUKey::Key_Dest) {
        m_stateMachine.setState(nav::CDUInputState::EnteringDestination);
        appendScratchpadMessage("ENTER DESTINATION AIRPORT:");
    } else if (key == nav::CDUKey::Key_Alt) {
        m_stateMachine.setState(nav::CDUInputState::EnteringAlternate);
        appendScratchpadMessage("ENTER ALTERNATE AIRPORT:");
    } else if (key == nav::CDUKey::Key_Exec) {
        processInput();
    } else {
        m_stateMachine.pressKey(key);
    }

    m_inputLineEdit->setText(QString::fromStdString(m_stateMachine.getInputBuffer()));
    QString statusText = QString("STATE: %1").arg(QString::fromStdString(m_stateMachine.getStateName()));
    m_statusLabel->setText(statusText);

    updateDisplay();
}

void CDUWidget::processInput() {
    auto state = m_stateMachine.getCurrentState();
    switch (state) {
        case nav::CDUInputState::EnteringOrigin:
            processOriginInput();
            break;
        case nav::CDUInputState::EnteringDestination:
            processDestinationInput();
            break;
        case nav::CDUInputState::EnteringAlternate:
            processAlternateInput();
            break;
        case nav::CDUInputState::EnteringRoute:
            processRouteInput();
            break;
        default:
            break;
    }
}

void CDUWidget::processOriginInput() {
    std::string input = m_stateMachine.getInputBuffer();
    if (input.empty()) {
        appendScratchpadMessage("ENTER ORIGIN ICAO CODE", true);
        return;
    }

    if (validateAirport(QString::fromStdString(input))) {
        m_routePlanner->setOrigin(input);
        appendScratchpadMessage(QString("ORIGIN SET: %1").arg(QString::fromStdString(input)));
        m_stateMachine.setState(nav::CDUInputState::Idle);
        emit flightPlanChanged(getCurrentFlightPlan());
    } else {
        appendScratchpadMessage(QString("AIRPORT NOT FOUND: %1").arg(QString::fromStdString(input)), true);
    }
    m_stateMachine.clearBuffer();
    m_inputLineEdit->clear();
}

void CDUWidget::processDestinationInput() {
    std::string input = m_stateMachine.getInputBuffer();
    if (input.empty()) {
        appendScratchpadMessage("ENTER DESTINATION ICAO CODE", true);
        return;
    }

    if (validateAirport(QString::fromStdString(input))) {
        m_routePlanner->setDestination(input);
        appendScratchpadMessage(QString("DESTINATION SET: %1").arg(QString::fromStdString(input)));
        m_stateMachine.setState(nav::CDUInputState::Idle);
        emit flightPlanChanged(getCurrentFlightPlan());
    } else {
        appendScratchpadMessage(QString("AIRPORT NOT FOUND: %1").arg(QString::fromStdString(input)), true);
    }
    m_stateMachine.clearBuffer();
    m_inputLineEdit->clear();
}

void CDUWidget::processAlternateInput() {
    std::string input = m_stateMachine.getInputBuffer();
    if (input.empty()) {
        appendScratchpadMessage("ENTER ALTERNATE ICAO CODE", true);
        return;
    }

    if (validateAirport(QString::fromStdString(input))) {
        m_routePlanner->setAlternate(input);
        appendScratchpadMessage(QString("ALTERNATE SET: %1").arg(QString::fromStdString(input)));
        m_stateMachine.setState(nav::CDUInputState::Idle);
        emit flightPlanChanged(getCurrentFlightPlan());
    } else {
        appendScratchpadMessage(QString("AIRPORT NOT FOUND: %1").arg(QString::fromStdString(input)), true);
    }
    m_stateMachine.clearBuffer();
    m_inputLineEdit->clear();
}

void CDUWidget::processRouteInput() {
    std::string input = m_stateMachine.getInputBuffer();
    if (input.empty()) {
        auto errors = m_routePlanner->validateRoute();
        if (errors.empty()) {
            executeRoute();
        } else {
            for (const auto& error : errors) {
                appendScratchpadMessage(QString::fromStdString(error), true);
            }
        }
        return;
    }

    if (validateWaypoint(QString::fromStdString(input))) {
        addRouteEntry(QString::fromStdString(input));
        m_stateMachine.clearBuffer();
        m_inputLineEdit->clear();
    } else {
        appendScratchpadMessage(QString("WAYPOINT NOT FOUND: %1").arg(QString::fromStdString(input)), true);
        m_stateMachine.clearBuffer();
        m_inputLineEdit->clear();
    }
}

void CDUWidget::addRouteEntry(const QString& identifier) {
    nav::RouteEntry entry;
    entry.identifier = identifier.toStdString();
    entry.airway = "DCT";
    m_routePlanner->addRouteEntry(entry);
    appendScratchpadMessage(QString("ADDED WAYPOINT: %1").arg(identifier));
    emit flightPlanChanged(getCurrentFlightPlan());
}

void CDUWidget::insertRouteEntry(const QString& identifier, int position) {
    nav::RouteEntry entry;
    entry.identifier = identifier.toStdString();
    entry.airway = "DCT";
    m_routePlanner->insertRouteEntry(static_cast<size_t>(position), entry);
    appendScratchpadMessage(QString("INSERTED WAYPOINT: %1 AT POS %2").arg(identifier).arg(position + 1));
    emit flightPlanChanged(getCurrentFlightPlan());
}

void CDUWidget::removeRouteEntry(int position) {
    if (m_routePlanner->removeRouteEntry(static_cast<size_t>(position))) {
        appendScratchpadMessage(QString("REMOVED WAYPOINT AT POS %1").arg(position + 1));
        emit flightPlanChanged(getCurrentFlightPlan());
    }
}

bool CDUWidget::validateWaypoint(const QString& identifier) {
    auto wp = m_database.findWaypoint(identifier.toStdString());
    if (wp.has_value()) return true;
    auto apt = m_database.findAirport(identifier.toStdString());
    return apt.has_value();
}

bool CDUWidget::validateAirport(const QString& identifier) {
    auto apt = m_database.findAirport(identifier.toStdString());
    return apt.has_value();
}

void CDUWidget::setDatabase(const arinc424::ARINC424Database& db) {
    m_database = db;
    m_routePlanner->setDatabase(db);
    appendScratchpadMessage(QString("ARINC 424 DATABASE LOADED: %1 WAYPOINTS, %2 AIRPORTS")
                                 .arg(db.waypoints.size())
                                 .arg(db.airports.size()));
}

void CDUWidget::setRoutePlanner(nav::RoutePlanner* planner) {
    if (m_routePlanner) {
        delete m_routePlanner;
    }
    m_routePlanner = planner;
}

nav::FlightPlan CDUWidget::getCurrentFlightPlan() const {
    return m_routePlanner->generateFlightPlan();
}

void CDUWidget::executeRoute() {
    auto errors = m_routePlanner->validateRoute();
    if (!errors.empty()) {
        appendScratchpadMessage("ROUTE VALIDATION FAILED:", true);
        for (const auto& error : errors) {
            appendScratchpadMessage(QString::fromStdString(error), true);
        }
        return;
    }

    auto plan = m_routePlanner->generateFlightPlan();
    if (plan.legs.empty()) {
        appendScratchpadMessage("NO LEGS IN FLIGHT PLAN", true);
        return;
    }

    appendScratchpadMessage(QString("ROUTE EXECUTED: %1 -> %2")
                                 .arg(QString::fromStdString(plan.origin))
                                 .arg(QString::fromStdString(plan.destination)));
    appendScratchpadMessage(QString("TOTAL DISTANCE: %1 NM / %2 KM")
                                 .arg(plan.totalDistanceNm, 0, 'f', 1)
                                 .arg(plan.totalDistanceKm, 0, 'f', 1));
    appendScratchpadMessage(QString("ESTIMATED TIME: %1 MIN")
                                 .arg(plan.estimatedTotalTime, 0, 'f', 0));

    m_stateMachine.setState(nav::CDUInputState::ExecutingPlan);
    emit routeExecuted(plan);
    emit flightPlanChanged(plan);
}

void CDUWidget::clearRoute() {
    m_routePlanner->clearRoute();
    m_stateMachine.setState(nav::CDUInputState::Idle);
    appendScratchpadMessage("ROUTE CLEARED");
    emit flightPlanChanged(getCurrentFlightPlan());
    updateDisplay();
}

void CDUWidget::loadDefaultRoute() {
    clearRoute();

    m_routePlanner->setOrigin("ZBAA");
    m_routePlanner->setDestination("ZSPD");

    nav::RouteEntry e1;
    e1.identifier = "W58";
    e1.airway = "A1";
    m_routePlanner->addRouteEntry(e1);

    nav::RouteEntry e2;
    e2.identifier = "PIKAS";
    e2.airway = "A1";
    m_routePlanner->addRouteEntry(e2);

    nav::RouteEntry e3;
    e3.identifier = "LADIX";
    e3.airway = "A1";
    m_routePlanner->addRouteEntry(e3);

    appendScratchpadMessage("DEFAULT ROUTE LOADED: ZBAA -> ZSPD");
    emit flightPlanChanged(getCurrentFlightPlan());
    updateDisplay();
}

void CDUWidget::appendScratchpadMessage(const QString& message, bool isError) {
    QListWidgetItem* item = new QListWidgetItem(message);
    if (isError) {
        item->setForeground(QBrush(CDU_RED_COLOR));
    } else {
        item->setForeground(QBrush(CDU_GREEN_COLOR));
    }
    m_scratchpadList->addItem(item);
    m_scratchpadList->scrollToBottom();

    while (m_scratchpadList->count() > 10) {
        delete m_scratchpadList->takeItem(0);
    }
}

void CDUWidget::updateDisplay() {
    renderPage();

    QString cursorSuffix = m_blinkState ? "█" : " ";
    m_inputLineEdit->setText(QString::fromStdString(m_stateMachine.getInputBuffer()) + cursorSuffix);
}

void CDUWidget::renderPage() {
    m_displayLines.clear();

    switch (m_currentPage) {
        case PAGE_IDLE:
            renderIdlePage();
            break;
        case PAGE_ROUTE:
            renderRoutePage();
            break;
        case PAGE_LEGS:
            renderLegsPage();
            break;
        case PAGE_INIT:
            renderInitPage();
            break;
        case PAGE_PERF:
            renderPerfPage();
            break;
        case PAGE_NAV:
            renderNavPage();
            break;
    }

    m_titleLabel->setText(QString("FMC - %1 PAGE").arg(QString::fromStdString(m_stateMachine.getStateName())));
}

void CDUWidget::renderIdlePage() {
    m_displayLines << "FLIGHT MANAGEMENT COMPUTER";
    m_displayLines << "============================";
    m_displayLines << "";
    m_displayLines << "SYSTEM READY";
    m_displayLines << "";
    m_displayLines << "SELECT A FUNCTION KEY:";
    m_displayLines << "  INIT  - INITIALIZATION";
    m_displayLines << "  ROUTE - ROUTE DEFINITION";
    m_displayLines << "  LEGS  - LEGS SUMMARY";
    m_displayLines << "  PROG  - PROGRESS";
    m_displayLines << "  NAV   - NAVIGATION";
    m_displayLines << "  PERF  - PERFORMANCE";
    m_displayLines << "";
    m_displayLines << "ARINC 424 DATABASE ACTIVE";

    m_scratchpadList->clear();
    for (const auto& line : m_displayLines) {
        appendScratchpadMessage(line);
    }
}

void CDUWidget::renderRoutePage() {
    auto plan = getCurrentFlightPlan();

    m_scratchpadList->clear();

    appendScratchpadMessage(">>> ROUTE DEFINITION <<<", false);
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage(QString("ORIGIN:      %1").arg(QString::fromStdString(m_routePlanner->getOrigin()).isEmpty()
                                                              ? "----"
                                                              : QString::fromStdString(m_routePlanner->getOrigin())));
    appendScratchpadMessage(QString("DESTINATION: %1").arg(QString::fromStdString(m_routePlanner->getDestination()).isEmpty()
                                                              ? "----"
                                                              : QString::fromStdString(m_routePlanner->getDestination())));
    appendScratchpadMessage(QString("ALTERNATE:   %1").arg(QString::fromStdString(m_routePlanner->getAlternate()).isEmpty()
                                                              ? "----"
                                                              : QString::fromStdString(m_routePlanner->getAlternate())));
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("ROUTE WAYPOINTS:", false);

    for (size_t i = 0; i < m_routePlanner->entryCount(); ++i) {
        auto entry = m_routePlanner->getEntry(i);
        if (entry.has_value()) {
            appendScratchpadMessage(QString("  %1. %2  (%3)")
                                         .arg(i + 1, 2)
                                         .arg(QString::fromStdString(entry->identifier))
                                         .arg(QString::fromStdString(entry->airway)));
        }
    }

    appendScratchpadMessage("----------------------------", false);
    if (!plan.legs.empty()) {
        appendScratchpadMessage(QString("TOTAL DISTANCE: %1 NM").arg(plan.totalDistanceNm, 0, 'f', 1));
        appendScratchpadMessage(QString("NUM LEGS:       %1").arg(plan.legs.size()));
    }

    appendScratchpadMessage("", false);
    auto state = m_stateMachine.getCurrentState();
    if (state == nav::CDUInputState::EnteringRoute) {
        appendScratchpadMessage("ENTER WAYPOINT ID + EXEC");
    } else if (state == nav::CDUInputState::EnteringOrigin) {
        appendScratchpadMessage("ENTER ORIGIN ICAO + EXEC");
    } else if (state == nav::CDUInputState::EnteringDestination) {
        appendScratchpadMessage("ENTER DEST ICAO + EXEC");
    }
}

void CDUWidget::renderLegsPage() {
    auto plan = getCurrentFlightPlan();

    m_scratchpadList->clear();
    appendScratchpadMessage(">>> FLIGHT PLAN LEGS <<<", false);
    appendScratchpadMessage("----------------------------", false);

    if (plan.legs.empty()) {
        appendScratchpadMessage("NO LEGS DEFINED", true);
        return;
    }

    appendScratchpadMessage(QString("%1 -> %2")
                                 .arg(QString::fromStdString(plan.origin))
                                 .arg(QString::fromStdString(plan.destination)));
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("SEQ WPT     DIST   TRK   CUM");

    for (size_t i = 0; i < plan.legs.size() && i < 8; ++i) {
        const auto& leg = plan.legs[i];
        QString distStr = (i == 0) ? "  ---" : QString("%1 NM").arg(leg.distanceFromOrigin, 5, 'f', 1);
        QString trkStr = (i == 0) ? " ---" : QString("%1°").arg(static_cast<int>(leg.trueTrack), 3);
        appendScratchpadMessage(QString("%1  %2   %3  %4  %5 NM")
                                     .arg(leg.sequenceNumber, 2)
                                     .arg(QString::fromStdString(leg.waypointIdentifier), -5)
                                     .arg(distStr)
                                     .arg(trkStr)
                                     .arg(leg.cumulativeDistance, 5, 'f', 0));
    }

    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage(QString("TOTAL: %1 NM / %2 MIN")
                                 .arg(plan.totalDistanceNm, 0, 'f', 1)
                                 .arg(plan.estimatedTotalTime, 0, 'f', 0));
}

void CDUWidget::renderInitPage() {
    m_scratchpadList->clear();
    appendScratchpadMessage(">>> INITIALIZATION PAGE <<<", false);
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("AIRCRAFT: B777-300ER");
    appendScratchpadMessage("FLIGHT NO: CCA1234");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("FLIGHT CREW:");
    appendScratchpadMessage("  CAPT: ********");
    appendScratchpadMessage("  FO:   ********");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("FUEL PLANNING:");
    appendScratchpadMessage("  BLOCK FUEL:  98,500 KG");
    appendScratchpadMessage("  TRIP FUEL:   62,300 KG");
    appendScratchpadMessage("  RESERVE:     12,000 KG");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("PRESS ROUTE TO CONTINUE");
}

void CDUWidget::renderPerfPage() {
    m_scratchpadList->clear();
    appendScratchpadMessage(">>> PERFORMANCE PAGE <<<", false);
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("CRUISE PROFILE:");
    appendScratchpadMessage("  ALTITUDE:  FL350");
    appendScratchpadMessage("  SPEED:     M0.84");
    appendScratchpadMessage("  TAS:       485 KT");
    appendScratchpadMessage("  GS:        470 KT");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("TAKE-OFF DATA:");
    appendScratchpadMessage("  RWY:       18L");
    appendScratchpadMessage("  V1:        145 KT");
    appendScratchpadMessage("  VR:        150 KT");
    appendScratchpadMessage("  V2:        155 KT");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("LANDING DATA:");
    appendScratchpadMessage("  RWY:       35R");
    appendScratchpadMessage("  VAPP:      140 KT");
    appendScratchpadMessage("  LANDING WT: 185,000 KG");
}

void CDUWidget::renderNavPage() {
    m_scratchpadList->clear();
    appendScratchpadMessage(">>> NAVIGATION STATUS <<<", false);
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("GPS:       ACTIVE (3D FIX)");
    appendScratchpadMessage("IRS 1:     ALIGNED");
    appendScratchpadMessage("IRS 2:     ALIGNED");
    appendScratchpadMessage("IRS 3:     ALIGNED");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("NAVAIDS TUNED:");
    appendScratchpadMessage("  VOR/DME:  PEK  114.70");
    appendScratchpadMessage("  ADF:      355 KHz");
    appendScratchpadMessage("----------------------------", false);
    appendScratchpadMessage("MAGNETIC VARIATION: -6.5°");
    appendScratchpadMessage("PRESSURE ALTITUDE: 35,000 FT");
    appendScratchpadMessage("TRUE ALTITUDE:    34,850 FT");

    auto plan = getCurrentFlightPlan();
    if (!plan.legs.empty()) {
        appendScratchpadMessage("----------------------------", false);
        appendScratchpadMessage(QString("NEXT WPT: %1").arg(QString::fromStdString(plan.legs[0].waypointIdentifier)));
    }
}

QString CDUWidget::formatCoordinates(double lat, double lon) const {
    char latHem = lat >= 0 ? 'N' : 'S';
    char lonHem = lon >= 0 ? 'E' : 'W';
    double absLat = std::abs(lat);
    double absLon = std::abs(lon);
    int latDeg = static_cast<int>(absLat);
    int latMin = static_cast<int>((absLat - latDeg) * 60);
    int lonDeg = static_cast<int>(absLon);
    int lonMin = static_cast<int>((absLon - lonDeg) * 60);

    return QString("%1%2°%3' %4%5°%6'")
        .arg(latHem).arg(latDeg, 2, 10, QChar('0')).arg(latMin, 2, 10, QChar('0'))
        .arg(lonHem).arg(lonDeg, 3, 10, QChar('0')).arg(lonMin, 2, 10, QChar('0'));
}

}
}
