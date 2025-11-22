#include <QApplication>
#include <QFile>
#include <QDebug>
#include "dbmanager.h"
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Sudbury Student Bank");
    app.setOrganizationName("A project by Group-9");

    // Global stylesheet: blue & black, gradients, modern look
    QString style = R"(
        QWidget {
            background-color: #050816;
            color: #E0E8FF;
            font-family: "Segoe UI", "Roboto", sans-serif;
            font-size: 13px;
        }
        QMainWindow {
            background-color: #050816;
        }
        #brandTitle {
            font-size: 26px;
            font-weight: 700;
            color: #6CA8FF;
        }
        #pageTitle {
            font-size: 20px;
            font-weight: 600;
            color: #9BB7FF;
        }
        #overviewMetric {
            font-size: 18px;
            font-weight: 600;
            color: #FFFFFF;
        }
        #overviewMetricSecondary {
            font-size: 16px;
            font-weight: 500;
            color: #9BB7FF;
        }
        #profileSummary {
            background-color: #060B20;
            border-radius: 12px;
            padding: 12px;
        }
        #profilePicFrame {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                              stop:0 #1E3C72, stop:1 #2A5298);
            border-radius: 12px;
            border: 2px solid #3C7BFA;
            color: #FFFFFF;
            font-size: 20px;
            font-weight: 600;
        }
        #statusLabel {
            color: #FF9B9B;
        }
        #linkButton {
            color: #8AB4FF;
            text-decoration: underline;
            border: none;
        }
        QLineEdit, QComboBox, QDateEdit {
            background-color: #060B20;
            border: 1px solid #1E3C72;
            border-radius: 6px;
            padding: 6px 8px;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border-color: #3C7BFA;
        }
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                              stop:0 #1E3C72, stop:1 #2A5298);
            border-radius: 8px;
            padding: 8px 16px;
            border: 1px solid #284B9B;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                              stop:0 #27468F, stop:1 #3560C0);
        }
        QPushButton:pressed {
            background-color: #162751;
        }
        QTabWidget::pane {
            border: 1px solid #1E3C72;
            border-radius: 6px;
        }
        QTabBar::tab {
            background-color: #050816;
            padding: 8px 16px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        QTabBar::tab:selected {
            background-color: #101A3A;
            color: #FFFFFF;
        }
        QHeaderView::section {
            background-color: #101A3A;
            padding: 4px;
            border: none;
        }
        QTableView {
            gridline-color: #1E3C72;
            selection-background-color: #1E3C72;
            selection-color: #FFFFFF;
        }
    )";
    app.setStyleSheet(style);

    if (!DBManager::init("bank.db")) {
        qWarning() << "Could not initialize database.";
    }

    LoginWindow login;
    MainWindow *mainWin = nullptr;

    QObject::connect(&login, &LoginWindow::loginSucceeded, [&](int userId){
        if (mainWin) {
            mainWin->close();
            delete mainWin;
        }
        mainWin = new MainWindow(userId);
        mainWin->show();
    });

    login.show();
    return app.exec();
}
