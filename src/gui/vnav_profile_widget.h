#pragma once

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QToolTip>
#include <QString>
#include <QPoint>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QColor>
#include <QVector>
#include <QPair>

#include "../core/vnav_solver.h"

namespace fmc {
namespace gui {

class VNAVProfileWidget : public QWidget {
    Q_OBJECT

public:
    explicit VNAVProfileWidget(QWidget* parent = nullptr);
    ~VNAVProfileWidget() override;

    void setVNAVProfile(const nav::VNAVProfile& profile);
    void clearProfile();
    nav::VNAVProfile getCurrentProfile() const { return m_profile; }

    QSize minimumSizeHint() const override { return QSize(400, 280); }
    QSize sizeHint() const override { return QSize(800, 400); }

signals:
    void waypointClicked(size_t legIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    nav::VNAVProfile m_profile;
    int m_marginLeft;
    int m_marginRight;
    int m_marginTop;
    int m_marginBottom;
    int m_plotAreaX;
    int m_plotAreaY;
    int m_plotAreaW;
    int m_plotAreaH;

    double m_xRangeNm;
    double m_yRangeFt;
    double m_xMinNm;
    double m_yMinFt;

    QVector<QPointF> m_profilePoints;
    QVector<QPair<QString, QPointF>> m_waypointLabels;

    void recalcPlotGeometry();
    QPointF toScreen(double distanceNm, double altitudeFt) const;
    QPair<double, double> fromScreen(const QPoint& pt) const;

    void drawBackground(QPainter& p);
    void drawGrid(QPainter& p);
    void drawAxes(QPainter& p);
    void drawProfileLine(QPainter& p);
    void drawWaypoints(QPainter& p);
    void drawConstraintMarkers(QPainter& p);
    void drawPhaseLabels(QPainter& p);
    void drawTOD_TOC(QPainter& p);

    static QColor phaseColor(nav::VerticalFlightPhase phase);
    static QString phaseLabel(nav::VerticalFlightPhase phase);
    static QString constraintTypeLabel(nav::AltitudeConstraintType type);

    size_t findNearestWaypoint(const QPoint& screenPt, double maxDistPx = 15.0) const;
};

}
}
