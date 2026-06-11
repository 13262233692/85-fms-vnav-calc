#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>
#include "gui/main_window.h"

int main(int argc, char* argv[]) {
    QSurfaceFormat format;
    format.setSamples(8);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("FMC Simulator");
    app.setOrganizationName("Aviation Simulation Systems");
    app.setApplicationVersion("1.0.0");

    int fontId = QFontDatabase::addApplicationFont(":/fonts/consola.ttf");
    if (fontId >= 0) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont defaultFont(fontFamily, 10);
        app.setFont(defaultFont);
    } else {
        QFont defaultFont("Consolas", 10);
        app.setFont(defaultFont);
    }

    if (QStyleFactory::keys().contains("Fusion")) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(15, 20, 35));
    palette.setColor(QPalette::WindowText, QColor(220, 230, 255));
    palette.setColor(QPalette::Base, QColor(10, 15, 25));
    palette.setColor(QPalette::AlternateBase, QColor(25, 35, 55));
    palette.setColor(QPalette::Text, QColor(220, 230, 255));
    palette.setColor(QPalette::Button, QColor(40, 50, 75));
    palette.setColor(QPalette::ButtonText, QColor(220, 230, 255));
    palette.setColor(QPalette::Highlight, QColor(0, 120, 180));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipBase, QColor(30, 40, 60));
    palette.setColor(QPalette::ToolTipText, QColor(220, 230, 255));
    palette.setColor(QPalette::Link, QColor(0, 200, 255));
    palette.setColor(QPalette::LinkVisited, QColor(150, 100, 200));
    app.setPalette(palette);

    app.setStyleSheet(R"(
        QToolTip {
            background-color: rgb(30, 40, 60);
            color: rgb(220, 230, 255);
            border: 1px solid rgb(60, 80, 120);
            padding: 4px;
            border-radius: 3px;
        }
        QScrollBar:horizontal {
            background-color: rgb(20, 30, 48);
            height: 12px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: rgb(60, 80, 120);
            min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");

    fmc::gui::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
