#pragma once

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>
#include <QStringList>
#include "../core/navigation.h"
#include "../core/arinc424_parser.h"

namespace fmc {
namespace gui {

class CDUWidget : public QWidget {
    Q_OBJECT

public:
    explicit CDUWidget(QWidget* parent = nullptr);
    ~CDUWidget() override = default;

    void setDatabase(const arinc424::ARINC424Database& db);
    void setRoutePlanner(nav::RoutePlanner* planner);
    nav::FlightPlan getCurrentFlightPlan() const;

    enum DisplayPage {
        PAGE_IDLE = 0,
        PAGE_ROUTE = 1,
        PAGE_LEGS = 2,
        PAGE_INIT = 3,
        PAGE_PERF = 4,
        PAGE_NAV = 5
    };

public slots:
    void handleKeyPress(nav::CDUKey key);
    void updateDisplay();
    void executeRoute();
    void clearRoute();
    void loadDefaultRoute();

signals:
    void flightPlanChanged(const nav::FlightPlan& plan);
    void routeExecuted(const nav::FlightPlan& plan);
    void waypointSelected(const std::string& waypointId);

private:
    nav::CDUInputStateMachine m_stateMachine;
    nav::RoutePlanner* m_routePlanner;
    arinc424::ARINC424Database m_database;
    DisplayPage m_currentPage;
    int m_scrollOffset;
    bool m_blinkState;
    QTimer* m_blinkTimer;

    QFrame* m_displayFrame;
    QVBoxLayout* m_displayLayout;
    QLabel* m_titleLabel;
    QLabel* m_statusLabel;
    QLabel* m_inputLineLabel;
    QListWidget* m_scratchpadList;
    QLineEdit* m_inputLineEdit;

    QFrame* m_keypadFrame;
    QGridLayout* m_keypadLayout;

    QStringList m_displayLines;

    void setupUI();
    void setupDisplayArea();
    void setupKeypad();
    void createKeyButton(const QString& label, nav::CDUKey key, int row, int col,
                        int rowSpan = 1, int colSpan = 1);
    void createFunctionKey(const QString& label, nav::CDUKey key, int row, int col);

    void renderPage();
    void renderIdlePage();
    void renderRoutePage();
    void renderLegsPage();
    void renderInitPage();
    void renderPerfPage();
    void renderNavPage();

    void appendScratchpadMessage(const QString& message, bool isError = false);
    void processInput();
    void processOriginInput();
    void processDestinationInput();
    void processAlternateInput();
    void processRouteInput();
    void processAltitudeInput();
    void processAirwayInput();
    void processSpeedInput();

    void addRouteEntry(const QString& identifier);
    void insertRouteEntry(const QString& identifier, int position);
    void removeRouteEntry(int position);

    bool validateWaypoint(const QString& identifier);
    bool validateAirport(const QString& identifier);

    QString formatCoordinates(double lat, double lon) const;

    static const QColor CDU_BG_COLOR;
    static const QColor CDU_DISPLAY_BG_COLOR;
    static const QColor CDU_TEXT_COLOR;
    static const QColor CDU_AMBER_COLOR;
    static const QColor CDU_GREEN_COLOR;
    static const QColor CDU_CYAN_COLOR;
    static const QColor CDU_MAGENTA_COLOR;
    static const QColor CDU_WHITE_COLOR;
    static const QColor CDU_RED_COLOR;

    static constexpr int DISPLAY_LINES = 14;
    static constexpr int LINE_CHARS = 24;
};

}
}
