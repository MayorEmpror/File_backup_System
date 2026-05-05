#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QDial>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <vector>
#include "../client/ClientSimulator.h"
#include "../core/backupmanager.h"
#include "LineChartWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QPushButton* startBtn;
    QPushButton* stopBtn;
    QTextEdit* logBox;
    QProgressBar* progressBar;
    QDial* speedDial;
    QLabel* speedLabel;
    QLabel* processedLabel;
    QLabel* failedLabel;
    QLabel* queueLabel;
    QLabel* workersLabel;
    QLabel* compressionLabel;
    QLineEdit* searchInput;
    QComboBox* sourceFilter;
    QTableWidget* requestsTable;
    LineChartWidget* compressionChart;
    LineChartWidget* queueChart;
    LineChartWidget* failureChart;

    ClientSimulator simulator;
    BackupManager manager;
    std::vector<Client*> clients;
    bool started;

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void log(const QString& msg);
    void addRequestRow(const BackupManager::RequestEvent& event);
    void applyTableFilter();

public slots:
    void onStartClicked();
    void onStopClicked();
    void onSpeedChanged(int value);
    void onFilterChanged();
};

#endif