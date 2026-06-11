#include "vnav_profile_widget.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QHelpEvent>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace fmc {
namespace gui {

VNAVProfileWidget::VNAVProfileWidget(QWidget* parent)
    : QWidget(parent),
      m_marginLeft(70),
      m_marginRight(20),
      m_marginTop(30),
      m_marginBottom(50),
      m_plotAreaX(0),
      m_plotAreaY(0),
      m_plotAreaW(0),
      m_plotAreaH(0),
      m_xRangeNm(0.0),
      m_yRangeFt(0.0),
      m_xMinNm(0.0),
      m_yMinFt(0.0) {
    setMinimumSize(minimumSizeHint());
    setMouseTracking(true);
}

VNAVProfileWidget::~VNAVProfileWidget() {
}

void VNAVProfileWidget::setVNAVProfile(const nav::VNAVProfile& profile) {
    m_profile = profile;
    recalcPlotGeometry();
    update();
}

void VNAVProfileWidget::clearProfile() {
    m_profile = nav::VNAVProfile();
    m_profilePoints.clear();
    m_waypointLabels.clear();
    update();
}

void VNAVProfileWidget::recalcPlotGeometry() {
    m_plotAreaX = m_marginLeft;
    m_plotAreaY = m_marginTop;
    m_plotAreaW = width() - m_marginLeft - m_marginRight;
    m_plotAreaH = height() - m_marginTop - m_marginBottom;

    if (m_plotAreaW <= 0) m_plotAreaW = 1;
    if (m_plotAreaH <= 0) m_plotAreaH = 1;

    if (m_profile.waypoints.empty()) {
        m_xRangeNm = 100.0;
        m_yRangeFt = 40000.0;
        m_xMinNm = 0.0;
        m_yMinFt = 0.0;
        return;
    }

    m_xMinNm = 0.0;
    m_xRangeNm = m_profile.totalDistanceNm;
    if (m_xRangeNm <= 0.0) m_xRangeNm = 1.0;

    double yMin = std::numeric_limits<double>::max();
    double yMax = 0.0;
    for (const auto& wp : m_profile.waypoints) {
        yMin = std::min(yMin, wp.plannedAltitudeFt);
        yMax = std::max(yMax, wp.plannedAltitudeFt);
        yMax = std::max(yMax, wp.maxAllowedAltitudeFt != std::numeric_limits<double>::infinity()
                              ? wp.maxAllowedAltitudeFt : 0.0);
    }

    m_yMinFt = std::max(0.0, yMin - 2000.0);
    m_yRangeFt = (yMax + 5000.0) - m_yMinFt;
    if (m_yRangeFt <= 0.0) m_yRangeFt = 1000.0;

    m_profilePoints.clear();
    m_waypointLabels.clear();
    for (const auto& wp : m_profile.waypoints) {
        QPointF sp = toScreen(wp.cumulativeDistanceNm, wp.plannedAltitudeFt);
        m_profilePoints.push_back(sp);
        m_waypointLabels.push_back(qMakePair(
            QString::fromStdString(wp.waypointIdentifier), sp));
    }
}

QPointF VNAVProfileWidget::toScreen(double distanceNm, double altitudeFt) const {
    double xNorm = (distanceNm - m_xMinNm) / m_xRangeNm;
    double yNorm = (altitudeFt - m_yMinFt) / m_yRangeFt;

    double sx = m_plotAreaX + xNorm * m_plotAreaW;
    double sy = m_plotAreaY + (1.0 - yNorm) * m_plotAreaH;

    return QPointF(sx, sy);
}

QPair<double, double> VNAVProfileWidget::fromScreen(const QPoint& pt) const {
    double xNorm = double(pt.x() - m_plotAreaX) / double(m_plotAreaW);
    double yNorm = 1.0 - double(pt.y() - m_plotAreaY) / double(m_plotAreaH);

    double distNm = m_xMinNm + xNorm * m_xRangeNm;
    double altFt = m_yMinFt + yNorm * m_yRangeFt;
    return qMakePair(distNm, altFt);
}

QColor VNAVProfileWidget::phaseColor(nav::VerticalFlightPhase phase) {
    switch (phase) {
        case nav::VerticalFlightPhase::Climb:    return QColor(0x00, 0xCC, 0x66);
        case nav::VerticalFlightPhase::Cruise:   return QColor(0x00, 0xFF, 0xFF);
        case nav::VerticalFlightPhase::Descent:  return QColor(0xFF, 0xAA, 0x00);
        case nav::VerticalFlightPhase::LevelOff: return QColor(0xFF, 0xFF, 0xFF);
        default: return QColor(0x80, 0x80, 0x80);
    }
}

QString VNAVProfileWidget::phaseLabel(nav::VerticalFlightPhase phase) {
    switch (phase) {
        case nav::VerticalFlightPhase::Climb:    return "CLB";
        case nav::VerticalFlightPhase::Cruise:   return "CRZ";
        case nav::VerticalFlightPhase::Descent:  return "DES";
        case nav::VerticalFlightPhase::LevelOff: return "LVL";
        default: return "???";
    }
}

QString VNAVProfileWidget::constraintTypeLabel(nav::AltitudeConstraintType type) {
    switch (type) {
        case nav::AltitudeConstraintType::At:        return "@";
        case nav::AltitudeConstraintType::AtOrAbove: return "A+";
        case nav::AltitudeConstraintType::AtOrBelow: return "B-";
        case nav::AltitudeConstraintType::Between:   return "A/B";
        default: return "";
    }
}

void VNAVProfileWidget::drawBackground(QPainter& p) {
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, QColor(0x08, 0x10, 0x18));
    bgGrad.setColorAt(1.0, QColor(0x00, 0x00, 0x00));
    p.fillRect(rect(), bgGrad);

    QRect plotRect(m_plotAreaX, m_plotAreaY, m_plotAreaW, m_plotAreaH);
    QLinearGradient plotGrad(plotRect.topLeft(), plotRect.bottomLeft());
    plotGrad.setColorAt(0.0, QColor(0x0A, 0x1A, 0x28));
    plotGrad.setColorAt(1.0, QColor(0x10, 0x20, 0x30));
    p.fillRect(plotRect, plotGrad);

    p.setPen(QPen(QColor(0x00, 0xFF, 0xFF, 0xC0), 1));
    p.drawRect(plotRect);
}

void VNAVProfileWidget::drawGrid(QPainter& p) {
    if (m_profile.waypoints.empty()) return;

    p.setPen(QPen(QColor(0x00, 0x66, 0x88, 0x60), 1, Qt::DashLine));

    int numYLines = 5;
    for (int i = 0; i <= numYLines; ++i) {
        double altFt = m_yMinFt + (double(i) / double(numYLines)) * m_yRangeFt;
        QPointF left = toScreen(m_xMinNm, altFt);
        QPointF right = toScreen(m_xMinNm + m_xRangeNm, altFt);
        p.drawLine(left, right);

        p.setPen(QColor(0x7F, 0xFF, 0xFF));
        QFont f = p.font();
        f.setPixelSize(10);
        p.setFont(f);
        QString label = QString::number(std::round(altFt / 1000.0) * 1000.0, 'f', 0);
        p.drawText(QRect(2, int(left.y()) - 8, m_marginLeft - 6, 16),
                   Qt::AlignRight | Qt::AlignVCenter, label + "ft");
        p.setPen(QPen(QColor(0x00, 0x66, 0x88, 0x60), 1, Qt::DashLine));
    }

    int numXLines = 5;
    for (int i = 0; i <= numXLines; ++i) {
        double distNm = m_xMinNm + (double(i) / double(numXLines)) * m_xRangeNm;
        QPointF top = toScreen(distNm, m_yMinFt + m_yRangeFt);
        QPointF bottom = toScreen(distNm, m_yMinFt);
        p.drawLine(top, bottom);

        p.setPen(QColor(0x7F, 0xFF, 0xFF));
        QFont f = p.font();
        f.setPixelSize(10);
        p.setFont(f);
        QString label = QString::number(distNm, 'f', 0);
        p.drawText(QRect(int(top.x()) - 30, m_plotAreaY + m_plotAreaH + 5, 60, 16),
                   Qt::AlignCenter, label + "NM");
        p.setPen(QPen(QColor(0x00, 0x66, 0x88, 0x60), 1, Qt::DashLine));
    }
}

void VNAVProfileWidget::drawAxes(QPainter& p) {
    p.setPen(QPen(QColor(0x00, 0xFF, 0xFF), 2));

    QFont labelFont;
    labelFont.setPixelSize(11);
    labelFont.setBold(true);
    p.setFont(labelFont);
    p.setPen(QColor(0x00, 0xFF, 0xFF));

    p.save();
    p.translate(15, m_plotAreaY + m_plotAreaH / 2);
    p.rotate(-90);
    p.drawText(QRect(-80, -10, 160, 20), Qt::AlignCenter, "ALTITUDE (FT)");
    p.restore();

    p.drawText(QRect(m_plotAreaX, m_plotAreaY + m_plotAreaH + 30,
                     m_plotAreaW, 18), Qt::AlignCenter, "DISTANCE (NM)");

    p.setPen(QColor(0x7F, 0xFF, 0x00));
    QFont titleFont;
    titleFont.setPixelSize(12);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.drawText(QRect(m_plotAreaX, 2, m_plotAreaW, 24), Qt::AlignCenter,
               "VNAV VERTICAL PROFILE - " + QString::number(m_profile.cruiseAltitudeFt, 'f', 0) + "FT CRZ");
}

void VNAVProfileWidget::drawProfileLine(QPainter& p) {
    if (m_profilePoints.size() < 2) return;

    QPainterPath path;
    path.moveTo(m_profilePoints[0]);

    for (int i = 1; i < m_profilePoints.size(); ++i) {
        path.lineTo(m_profilePoints[i]);
    }

    QPainterPath fillPath = path;
    fillPath.lineTo(toScreen(m_xMinNm + m_xRangeNm, m_yMinFt));
    fillPath.lineTo(toScreen(m_xMinNm, m_yMinFt));
    fillPath.closeSubpath();

    QColor fillBase(0x00, 0xFF, 0xFF, 0x20);
    QBrush fillBrush(fillBase);
    p.fillPath(fillPath, fillBrush);

    QPen linePen(QColor(0x00, 0xFF, 0xFF), 3);
    linePen.setJoinStyle(Qt::RoundJoin);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    p.drawPath(path);

    for (int i = 1; i < m_profilePoints.size(); ++i) {
        const auto& wp = m_profile.waypoints[i];
        QColor c = phaseColor(wp.phase);
        c.setAlpha(220);
        p.setPen(QPen(c, 2));
        p.drawLine(m_profilePoints[i-1], m_profilePoints[i]);
    }
}

void VNAVProfileWidget::drawWaypoints(QPainter& p) {
    for (int i = 0; i < m_profilePoints.size(); ++i) {
        const auto& wp = m_profile.waypoints[i];
        const QPointF& sp = m_profilePoints[i];

        double r = wp.constraintViolated ? 8.0 : 6.0;
        QColor c = wp.constraintViolated ? QColor(0xFF, 0x40, 0x40)
                   : (wp.originalConstraint.type != nav::AltitudeConstraintType::None
                      ? QColor(0xFF, 0xD0, 0x00) : QColor(0x00, 0xFF, 0xFF));

        p.setBrush(c);
        p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 1));
        p.drawEllipse(sp, r, r);

        p.setPen(QColor(0xFF, 0xFF, 0xFF));
        QFont f = p.font();
        f.setPixelSize(9);
        p.setFont(f);

        QString altLabel = QString::number(std::round(wp.plannedAltitudeFt / 100.0) * 100.0, 'f', 0);
        QRectF labelRect(sp.x() + 10, sp.y() - 22, 80, 14);
        p.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, altLabel);

        p.setPen(QColor(0xA0, 0xFF, 0xFF));
        QString nameLabel = m_waypointLabels[i].first;
        QRectF nameRect(sp.x() - 30, sp.y() + 10, 60, 14);
        p.drawText(nameRect, Qt::AlignCenter, nameLabel);
    }
}

void VNAVProfileWidget::drawConstraintMarkers(QPainter& p) {
    for (const auto& wp : m_profile.waypoints) {
        if (wp.originalConstraint.type == nav::AltitudeConstraintType::None) continue;

        QPointF basePt = toScreen(wp.cumulativeDistanceNm, 0);
        basePt.setY(m_plotAreaY);

        p.setPen(QPen(QColor(0xFF, 0xD0, 0x00, 0x80), 1, Qt::DotLine));

        if (wp.minAllowedAltitudeFt != -std::numeric_limits<double>::infinity() &&
            wp.originalConstraint.type != nav::AltitudeConstraintType::AtOrBelow) {
            QPointF lp = toScreen(wp.cumulativeDistanceNm, wp.minAllowedAltitudeFt);
            if (lp.y() >= m_plotAreaY && lp.y() <= m_plotAreaY + m_plotAreaH) {
                p.setPen(QPen(QColor(0x00, 0xFF, 0x66, 0xA0), 2, Qt::DashLine));
                p.drawLine(QPointF(m_plotAreaX, lp.y()),
                           QPointF(m_plotAreaX + m_plotAreaW, lp.y()));

                p.setPen(QColor(0x00, 0xFF, 0x66));
                QFont f; f.setPixelSize(9); p.setFont(f);
                p.drawText(QRect(m_plotAreaX + m_plotAreaW - 70, int(lp.y()) - 7, 70, 14),
                           Qt::AlignRight, "ABV " + QString::number(wp.minAllowedAltitudeFt, 'f', 0));
            }
        }

        if (wp.maxAllowedAltitudeFt != std::numeric_limits<double>::infinity() &&
            wp.originalConstraint.type != nav::AltitudeConstraintType::AtOrAbove) {
            QPointF lp = toScreen(wp.cumulativeDistanceNm, wp.maxAllowedAltitudeFt);
            if (lp.y() >= m_plotAreaY && lp.y() <= m_plotAreaY + m_plotAreaH) {
                p.setPen(QPen(QColor(0xFF, 0xAA, 0x00, 0xA0), 2, Qt::DashLine));
                p.drawLine(QPointF(m_plotAreaX, lp.y()),
                           QPointF(m_plotAreaX + m_plotAreaW, lp.y()));

                p.setPen(QColor(0xFF, 0xAA, 0x00));
                QFont f; f.setPixelSize(9); p.setFont(f);
                p.drawText(QRect(m_plotAreaX + m_plotAreaW - 70, int(lp.y()) - 7, 70, 14),
                           Qt::AlignRight, "BLW " + QString::number(wp.maxAllowedAltitudeFt, 'f', 0));
            }
        }
    }
}

void VNAVProfileWidget::drawPhaseLabels(QPainter& p) {
    if (m_profile.waypoints.size() < 2) return;

    for (size_t i = 1; i < m_profile.waypoints.size(); ++i) {
        const auto& wp = m_profile.waypoints[i];
        if (wp.phase == nav::VerticalFlightPhase::Unknown) continue;

        double midDist = (m_profile.waypoints[i-1].cumulativeDistanceNm +
                          wp.cumulativeDistanceNm) / 2.0;
        double midAlt = (m_profile.waypoints[i-1].plannedAltitudeFt +
                         wp.plannedAltitudeFt) / 2.0;
        QPointF mp = toScreen(midDist, midAlt);

        QColor c = phaseColor(wp.phase);
        QRectF bgRect(mp.x() - 18, mp.y() - 40, 36, 16);

        p.fillRect(bgRect, QColor(c.red(), c.green(), c.blue(), 0x80));
        p.setPen(QPen(c, 1));
        p.drawRect(bgRect);

        p.setPen(Qt::black);
        QFont f; f.setPixelSize(9); f.setBold(true); p.setFont(f);
        p.drawText(bgRect, Qt::AlignCenter, phaseLabel(wp.phase));
    }
}

void VNAVProfileWidget::drawTOD_TOC(QPainter& p) {
    if (m_profile.waypoints.empty()) return;

    if (m_profile.topOfClimbDistanceNm > 0.0 &&
        m_profile.topOfClimbDistanceNm < m_profile.totalDistanceNm) {
        double tocAlt = 0.0;
        for (size_t i = 1; i < m_profile.waypoints.size(); ++i) {
            if (m_profile.waypoints[i].cumulativeDistanceNm >= m_profile.topOfClimbDistanceNm) {
                tocAlt = m_profile.waypoints[i].plannedAltitudeFt;
                break;
            }
        }
        if (tocAlt <= 0) tocAlt = m_profile.cruiseAltitudeFt;

        QPointF toc = toScreen(m_profile.topOfClimbDistanceNm, tocAlt);
        p.setPen(QPen(QColor(0x00, 0xFF, 0x66), 2, Qt::DashLine));
        p.drawLine(QPointF(toc.x(), m_plotAreaY), QPointF(toc.x(), m_plotAreaY + m_plotAreaH));

        p.setPen(QColor(0x00, 0xFF, 0x66));
        QFont f; f.setPixelSize(10); f.setBold(true); p.setFont(f);
        p.drawText(QRect(int(toc.x()) - 15, m_plotAreaY - 18, 30, 16),
                   Qt::AlignCenter, "TOC");
    }

    if (m_profile.topOfDescentDistanceNm < m_profile.totalDistanceNm &&
        m_profile.topOfDescentDistanceNm > 0.0) {
        double todAlt = m_profile.cruiseAltitudeFt;
        QPointF tod = toScreen(m_profile.topOfDescentDistanceNm, todAlt);
        p.setPen(QPen(QColor(0xFF, 0xAA, 0x00), 2, Qt::DashLine));
        p.drawLine(QPointF(tod.x(), m_plotAreaY), QPointF(tod.x(), m_plotAreaY + m_plotAreaH));

        p.setPen(QColor(0xFF, 0xAA, 0x00));
        QFont f; f.setPixelSize(10); f.setBold(true); p.setFont(f);
        p.drawText(QRect(int(tod.x()) - 15, m_plotAreaY - 18, 30, 16),
                   Qt::AlignCenter, "TOD");
    }
}

void VNAVProfileWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    drawBackground(p);
    if (m_profile.waypoints.empty()) {
        p.setPen(QColor(0x80, 0xFF, 0xFF));
        QFont f; f.setPixelSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter,
                   "NO VNAV PROFILE AVAILABLE\n\n"
                   "Please execute a route with altitude constraints\n"
                   "to generate the vertical navigation profile.");
        return;
    }

    drawGrid(p);
    drawConstraintMarkers(p);
    drawTOD_TOC(p);
    drawProfileLine(p);
    drawWaypoints(p);
    drawPhaseLabels(p);
    drawAxes(p);

    if (!m_profile.warnings.empty() || m_profile.status == nav::VNAVSolverStatus::ConstraintInfeasible) {
        QColor barColor = m_profile.status == nav::VNAVSolverStatus::ConstraintInfeasible
                          ? QColor(0xFF, 0x40, 0x40, 0xE0)
                          : QColor(0xFF, 0x80, 0x00, 0xE0);
        p.fillRect(QRect(0, height() - 24, width(), 24), barColor);
        p.setPen(Qt::white);
        QFont f; f.setPixelSize(10); f.setBold(true); p.setFont(f);
        QString msg = m_profile.status == nav::VNAVSolverStatus::ConstraintInfeasible
                      ? "INFEASIBLE: "
                      : "WARNING: ";
        msg += QString::fromStdString(m_profile.warnings.empty()
                                      ? "Constraints relaxed"
                                      : m_profile.warnings[0]);
        if (m_profile.warnings.size() > 1) {
            msg += QString(" (+%1 more)").arg(m_profile.warnings.size() - 1);
        }
        p.drawText(QRect(8, height() - 24, width() - 16, 24), Qt::AlignLeft | Qt::AlignVCenter, msg);
    }
}

void VNAVProfileWidget::resizeEvent(QResizeEvent* /*event*/) {
    recalcPlotGeometry();
}

size_t VNAVProfileWidget::findNearestWaypoint(const QPoint& screenPt, double maxDistPx) const {
    size_t bestIdx = size_t(-1);
    double bestDist = maxDistPx * maxDistPx;

    for (int i = 0; i < m_profilePoints.size(); ++i) {
        double dx = screenPt.x() - m_profilePoints[i].x();
        double dy = screenPt.y() - m_profilePoints[i].y();
        double d2 = dx * dx + dy * dy;
        if (d2 < bestDist) {
            bestDist = d2;
            bestIdx = size_t(i);
        }
    }
    return bestIdx;
}

void VNAVProfileWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    size_t idx = findNearestWaypoint(event->pos());
    if (idx != size_t(-1)) {
        emit waypointClicked(idx);
    }
}

bool VNAVProfileWidget::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent* help = static_cast<QHelpEvent*>(event);
        size_t idx = findNearestWaypoint(help->pos(), 20.0);
        if (idx != size_t(-1) && idx < m_profile.waypoints.size()) {
            const auto& wp = m_profile.waypoints[idx];
            QString tt = QString(
                "<div style='font-family:Consolas,monospace; font-size:9pt;'>"
                "<b style='color:#00FFFF;'>%1</b><br>"
                "<table>"
                "<tr><td style='color:#7FFFD4;'>Seq:</td><td align='right'>%2</td></tr>"
                "<tr><td style='color:#7FFFD4;'>Distance:</td><td align='right'>%3 NM</td></tr>"
                "<tr><td style='color:#7FFFD4;'>Altitude:</td><td align='right'>%4 ft</td></tr>"
                "<tr><td style='color:#7FFFD4;'>Phase:</td><td align='right' style='color:%5;'>%6</td></tr>"
                "<tr><td style='color:#7FFFD4;'>V/S:</td><td align='right'>%7 ft/min</td></tr>"
                "<tr><td style='color:#7FFFD4;'>FPA:</td><td align='right'>%8 deg</td></tr>"
                "</table>")
                .arg(QString::fromStdString(wp.waypointIdentifier))
                .arg(wp.legIndex + 1)
                .arg(wp.cumulativeDistanceNm, 0, 'f', 1)
                .arg(wp.plannedAltitudeFt, 0, 'f', 0)
                .arg(phaseColor(wp.phase).name())
                .arg(phaseLabel(wp.phase))
                .arg(wp.verticalSpeedFpm, 0, 'f', 0)
                .arg(wp.flightPathAngleDeg, 0, 'f', 2);

            if (wp.originalConstraint.type != nav::AltitudeConstraintType::None) {
                tt += QString("<div style='margin-top:4px; "
                              "color:%1; border-top:1px solid #444; padding-top:4px;'>"
                              "<b>CONSTRAINT:</b> %2")
                      .arg(wp.constraintViolated ? "#FF4040" : "#FFD000")
                      .arg(constraintTypeLabel(wp.originalConstraint.type));
                if (wp.originalConstraint.type == nav::AltitudeConstraintType::At)
                    tt += QString(" %1 ft").arg(wp.originalConstraint.altitudeAtFt, 0, 'f', 0);
                else if (wp.originalConstraint.type == nav::AltitudeConstraintType::AtOrAbove)
                    tt += QString(" ABOVE %1 ft").arg(wp.originalConstraint.altitudeAboveFt, 0, 'f', 0);
                else if (wp.originalConstraint.type == nav::AltitudeConstraintType::AtOrBelow)
                    tt += QString(" BELOW %1 ft").arg(wp.originalConstraint.altitudeBelowFt, 0, 'f', 0);
                else if (wp.originalConstraint.type == nav::AltitudeConstraintType::Between)
                    tt += QString(" %1-%2 ft")
                          .arg(wp.originalConstraint.altitudeAboveFt, 0, 'f', 0)
                          .arg(wp.originalConstraint.altitudeBelowFt, 0, 'f', 0);
                tt += "</div>";
            }

            if (wp.constraintViolated && !wp.violationMessage.empty()) {
                tt += QString("<div style='margin-top:4px; color:#FF4040;'>"
                              "<b>VIOLATION:</b> %1</div>")
                      .arg(QString::fromStdString(wp.violationMessage));
            }
            tt += "</div>";

            QToolTip::showText(help->globalPos(), tt, this, QRect(), 5000);
            return true;
        } else {
            QToolTip::hideText();
        }
    }
    return QWidget::event(event);
}

}
}
