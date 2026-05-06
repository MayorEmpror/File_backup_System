#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Base class for main application windows in Qt.
#include <QMainWindow>
// Button widgets for start/stop controls.
#include <QPushButton>
// Multi-line text widget for logs.
#include <QTextEdit>
// Progress bar widget for overall progress display.
#include <QProgressBar>
// Dial widget for throughput control.
#include <QDial>
// Text label widget used for titles and KPI values.
#include <QLabel>
// Single-line text input for filtering.
#include <QLineEdit>
// Dropdown selection widget for source filtering.
#include <QComboBox>
// Table widget for displaying per-request rows.
#include <QTableWidget>
// Standard vector container for owning client pointers.
#include <vector>
// ClientSimulator drives generation of backup requests from clients.
#include "../client/ClientSimulator.h"
// BackupManager processes requests concurrently and emits callbacks.
#include "../core/backupmanager.h"
// LineChartWidget renders simple line charts for KPIs.
#include "LineChartWidget.h"

// Main dashboard window that wires UI controls to the simulator and backup manager.
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    // Start button that triggers system start.
    QPushButton* startBtn;
    // Stop button that triggers system shutdown.
    QPushButton* stopBtn;
    // Text area used to display log messages.
    QTextEdit* logBox;
    // Progress bar showing overall compression/progress percentage.
    QProgressBar* progressBar;
    // Dial used to control request pacing / throughput.
    QDial* speedDial;
    // Label above/near the dial explaining throughput control.
    QLabel* speedLabel;
    // KPI label showing processed requests count.
    QLabel* processedLabel;
    // KPI label showing failures count.
    QLabel* failedLabel;
    // KPI label showing queue depth.
    QLabel* queueLabel;
    // KPI label showing active worker count.
    QLabel* workersLabel;
    // KPI label showing compression percentage.
    QLabel* compressionLabel;
    // Search/filter input for table filtering.
    QLineEdit* searchInput;
    // Dropdown filter for "source" column.
    QComboBox* sourceFilter;
    // Table showing detailed per-request events.
    QTableWidget* requestsTable;
    // Chart tracking compression percentage over time.
    LineChartWidget* compressionChart;
    // Chart tracking queue depth pressure over time.
    LineChartWidget* queueChart;
    // Chart tracking failure rate over time.
    LineChartWidget* failureChart;

    // Simulator that generates backup requests from client threads.
    ClientSimulator simulator;
    // Manager that processes requests using worker threads and disk slots.
    BackupManager manager;
    // Client objects owned by the UI for the simulation lifetime.
    std::vector<Client*> clients;
    // Tracks whether the system is currently started (to prevent double start/stop).
    bool started;

public:
    // Construct the UI, build widgets/layout, and wire callbacks.
    MainWindow(QWidget* parent = nullptr);
    // Ensure simulation stops and clients are destroyed.
    ~MainWindow();

    // Append a message to the log view.
    void log(const QString& msg);
    // Add a row to the requests table describing one handled request.
    void addRequestRow(const BackupManager::RequestEvent& event);
    // Apply filtering rules to show/hide table rows based on UI inputs.
    void applyTableFilter();

public slots:
    // Slot called when the Start button is clicked.
    void onStartClicked();
    // Slot called when the Stop button is clicked.
    void onStopClicked();
    // Slot called when the throughput dial value changes.
    void onSpeedChanged(int value);
    // Slot called when filter inputs change (search text/source dropdown).
    void onFilterChanged();
};

#endif