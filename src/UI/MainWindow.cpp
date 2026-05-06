#include "MainWindow.h"
// Layout containers for arranging widgets vertically/horizontally.
#include <QVBoxLayout>
#include <QHBoxLayout>
// Frame widgets used for visual grouping and styling hooks.
#include <QFrame>
// Base widget type for containers.
#include <QWidget>
// Used to safely invoke UI updates from non-UI threads.
#include <QMetaObject>
// Qt string type used throughout the UI.
#include <QString>
// Used to control QTableWidget header sizing and appearance.
#include <QHeaderView>
// Used to display timestamps in the request table.
#include <QDateTime>
// Splitter widget used to create resizable panes.
#include <QSplitter>
// Scroll area used to make the header bar horizontally scrollable.
#include <QScrollArea>
// Size policy configuration for responsive layouts.
#include <QSizePolicy>
// Used to configure selection/edit behavior of item views.
#include <QAbstractItemView>
// Brush/color used for table item highlighting.
#include <QBrush>
// Font support for any widget-specific font tweaks.
#include <QFont>

namespace {

void setKpiRichText(QLabel* label, const QString& title, const QString& value) {
    // Ensure the label interprets its content as rich text (HTML).
    label->setTextFormat(Qt::RichText);
    // Set a two-line HTML snippet: small uppercase title + large monospace value.
    label->setText(
        QStringLiteral("<div style=\"color:#7a8088;font-size:10px;letter-spacing:2px;text-transform:uppercase;\">%1</div>"
                       "<div style=\"color:#e8eaed;font-size:22px;font-weight:600;font-family:'Consolas','Courier New',monospace;margin-top:6px;\">%2</div>")
            .arg(title, value));
}

QString industrialStylesheet() {
    // Return a consolidated stylesheet string that gives the app an "industrial" dark dashboard look.
    // The individual selector rules are embedded as a single concatenated QStringLiteral for performance.
    return QStringLiteral(
        "QMainWindow { background-color: #121418; }"
        "QWidget#centralRoot { background-color: #121418; color: #d7d9de; }"
        "QFrame#topStrip { background-color: #1a1e26; border-bottom: 1px solid #2d323c; }"
        "QLabel#titleLabel { font-size: 20px; font-weight: 700; color: #e8eaed; letter-spacing: 0.5px; }"
        "QLabel#subtitle { color: #7a8088; font-size: 11px; }"
        "QFrame#kpiCard { background-color: #1e2229; border: 1px solid #2d323c; border-radius: 4px; }"
        "QFrame#kpiCard QLabel { background: transparent; }"
        "QFrame#dialShell { background-color: #1e2229; border: 1px solid #2d323c; border-radius: 4px; }"
        "QPushButton#startBtn { background-color: #2d4a22; color: #c9e8b0; border: 1px solid #3d6a32; border-radius: 4px; padding: 8px 18px; font-weight: 600; }"
        "QPushButton#startBtn:hover { background-color: #3a5f2d; border-color: #4a7a3a; }"
        "QPushButton#startBtn:pressed { background-color: #243818; }"
        "QPushButton#stopBtn { background-color: #3a2420; color: #f0c4b8; border: 1px solid #6b3a2a; border-radius: 4px; padding: 8px 18px; font-weight: 600; }"
        "QPushButton#stopBtn:hover { background-color: #4d3028; border-color: #8a4a38; }"
        "QPushButton#stopBtn:pressed { background-color: #2a1814; }"
        "QLineEdit, QComboBox { background-color: #1e2229; color: #e8eaed; border: 1px solid #3d424d; border-radius: 4px; padding: 8px 12px; min-height: 20px; }"
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #5a6570; }"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox QAbstractItemView { background-color: #1e2229; color: #e8eaed; selection-background-color: #3d424d; }"
        "QProgressBar { border: 1px solid #2d323c; border-radius: 3px; background-color: #1a1e26; min-height: 18px; text-align: center; color: #c9a227; font-weight: 600; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #6b5a2a, stop:1 #c9a227); border-radius: 2px; }"
        "QTableWidget { background-color: #1a1e26; alternate-background-color: #161a20; color: #d7d9de; "
        "gridline-color: #2d323c; border: 1px solid #2d323c; border-radius: 4px; }"
        "QTableWidget::item:selected { background-color: #2a3140; color: #e8eaed; }"
        "QHeaderView::section { background-color: #22262f; color: #9aa0a8; border: none; border-bottom: 2px solid #c9a227; "
        "padding: 10px 8px; font-weight: 700; font-size: 11px; letter-spacing: 0.5px; }"
        "QTextEdit { background-color: #14171c; color: #9aa3af; border: 1px solid #2d323c; border-radius: 4px; "
        "font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; padding: 8px; }"
        "QSplitter::handle { background-color: #2d323c; height: 4px; width: 4px; }"
        "QSplitter::handle:hover { background-color: #c9a227; }"
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #1a1e26; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #3d424d; min-height: 24px; border-radius: 2px; }"
        "QScrollBar::handle:vertical:hover { background: #5a6570; }"
        "QScrollBar:horizontal { background: #1a1e26; height: 10px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #3d424d; min-width: 24px; border-radius: 2px; }"
        "QDial { background-color: transparent; }");
}

QFrame* makeKpiCard(QLabel* label) {
    // Create a frame to act as a "card" around a KPI label.
    QFrame* card = new QFrame();
    // Set object name so the stylesheet can target this frame.
    card->setObjectName(QStringLiteral("kpiCard"));
    // Lay out the label with padding inside the card.
    QVBoxLayout* c = new QVBoxLayout(card);
    c->setContentsMargins(14, 12, 14, 14);
    c->addWidget(label);
    // Return the assembled card widget.
    return card;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    // Initialize base class, configure the backup manager, and set initial run state.
    : QMainWindow(parent), manager(4, 2), started(false) {

    // Root central widget that holds all other content.
    QWidget* central = new QWidget(this);
    // Name is used by the stylesheet to apply root-level styling.
    central->setObjectName(QStringLiteral("centralRoot"));
    // Outer layout for the entire window content.
    QVBoxLayout* outer = new QVBoxLayout(central);
    outer->setSpacing(0);
    outer->setContentsMargins(0, 0, 0, 0);

    // Construct the Start and Stop control buttons.
    startBtn = new QPushButton(QStringLiteral("START"));
    startBtn->setObjectName(QStringLiteral("startBtn"));
    stopBtn = new QPushButton(QStringLiteral("STOP"));
    stopBtn->setObjectName(QStringLiteral("stopBtn"));
    // Construct the log box, progress bar, and throughput dial.
    logBox = new QTextEdit();
    progressBar = new QProgressBar();
    speedDial = new QDial();
    // Label for the throughput dial (styled as subtitle).
    speedLabel = new QLabel(QStringLiteral("THROUGHPUT"));
    speedLabel->setObjectName(QStringLiteral("subtitle"));
    // KPI label widgets that will be rendered as rich text.
    processedLabel = new QLabel();
    failedLabel = new QLabel();
    queueLabel = new QLabel();
    workersLabel = new QLabel();
    compressionLabel = new QLabel();
    // Filtering widgets for the requests table.
    searchInput = new QLineEdit();
    sourceFilter = new QComboBox();
    // Table for per-request events plus line charts for live trends.
    requestsTable = new QTableWidget();
    compressionChart = new LineChartWidget();
    queueChart = new LineChartWidget();
    failureChart = new LineChartWidget();

    // Initialize KPI labels with default values.
    setKpiRichText(processedLabel, QStringLiteral("Total requests"), QStringLiteral("0"));
    setKpiRichText(failedLabel, QStringLiteral("Failures"), QStringLiteral("0"));
    setKpiRichText(queueLabel, QStringLiteral("Queue depth"), QStringLiteral("0"));
    setKpiRichText(workersLabel, QStringLiteral("Active workers"), QStringLiteral("0"));
    setKpiRichText(compressionLabel, QStringLiteral("Compression"), QStringLiteral("0%"));

    // Configure the log box as output-only and give it a reasonable minimum height.
    logBox->setReadOnly(true);
    logBox->setMinimumHeight(96);
    logBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Configure progress bar range and display formatting.
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat(QStringLiteral("%p%"));
    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Configure throughput dial (1..10) with default of 5 and visible notches.
    speedDial->setRange(1, 10);
    speedDial->setValue(5);
    speedDial->setNotchesVisible(true);
    speedDial->setFixedSize(72, 72);
    speedDial->setWrapping(false);

    // Configure filter widgets: placeholder for text search and options for source filter.
    searchInput->setPlaceholderText(QStringLiteral("Filter by file or client ID…"));
    searchInput->setMinimumWidth(200);
    sourceFilter->addItems(QStringList() << QStringLiteral("All Sources")
                                          << QStringLiteral("Booking.com") << QStringLiteral("Expedia")
                                          << QStringLiteral("airbnb"));

    // Configure the requests table columns and headers.
    requestsTable->setColumnCount(9);
    requestsTable->setHorizontalHeaderLabels(
        QStringList() << QStringLiteral("ID") << QStringLiteral("Client") << QStringLiteral("File")
                      << QStringLiteral("Size") << QStringLiteral("Stored") << QStringLiteral("Source")
                      << QStringLiteral("Status") << QStringLiteral("Compression") << QStringLiteral("Time"));
    requestsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    requestsTable->horizontalHeader()->setStretchLastSection(true);
    requestsTable->horizontalHeader()->setDefaultSectionSize(110);
    requestsTable->verticalHeader()->setVisible(false);
    requestsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    requestsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    requestsTable->setAlternatingRowColors(true);
    requestsTable->setShowGrid(true);
    requestsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    requestsTable->setMinimumHeight(180);

    // Configure chart titles and line colors for visual distinction.
    compressionChart->setTitle(QStringLiteral("Compression ratio"));
    compressionChart->setLineColor(QColor(QStringLiteral("#c9a227")));
    queueChart->setTitle(QStringLiteral("Queue pressure"));
    queueChart->setLineColor(QColor(QStringLiteral("#7eb8da")));
    failureChart->setTitle(QStringLiteral("Failure rate"));
    failureChart->setLineColor(QColor(QStringLiteral("#c97a5a")));

    // Top strip frame containing application title and version indicator.
    QFrame* topStrip = new QFrame();
    topStrip->setObjectName(QStringLiteral("topStrip"));
    QHBoxLayout* topStripLay = new QHBoxLayout(topStrip);
    topStripLay->setContentsMargins(14, 8, 14, 8);

    // Title and version labels displayed in the top strip.
    QLabel* title = new QLabel(QStringLiteral("BACKUP CONTROL — OPERATIONS"));
    title->setObjectName(QStringLiteral("titleLabel"));
    QLabel* ver = new QLabel(QStringLiteral("DFBMS v1"));
    ver->setObjectName(QStringLiteral("subtitle"));
    topStripLay->addWidget(title);
    topStripLay->addStretch();
    topStripLay->addWidget(ver);

    // Scroll area used to keep the header bar usable on smaller window widths.
    QScrollArea* headerScroll = new QScrollArea();
    headerScroll->setWidgetResizable(true);
    headerScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    headerScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    headerScroll->setFrameShape(QFrame::NoFrame);
    headerScroll->setMaximumHeight(100);
    headerScroll->setMinimumHeight(72);

    // Inner widget that actually holds the header controls inside the scroll area.
    QWidget* headerInner = new QWidget();
    QHBoxLayout* headerBar = new QHBoxLayout(headerInner);
    headerBar->setSpacing(12);
    headerBar->setContentsMargins(14, 6, 14, 6);
    headerBar->addWidget(searchInput, 2);
    headerBar->addWidget(sourceFilter, 1);
    headerBar->addWidget(startBtn, 0);
    headerBar->addWidget(stopBtn, 0);
    headerBar->addSpacing(8);
    headerBar->addWidget(speedLabel, 0);
    // Visual shell frame around the dial to match the card style.
    QFrame* dialShell = new QFrame();
    dialShell->setObjectName(QStringLiteral("dialShell"));
    QVBoxLayout* dialLay = new QVBoxLayout(dialShell);
    dialLay->setContentsMargins(8, 6, 8, 6);
    dialLay->addWidget(speedDial, 0, Qt::AlignCenter);
    headerBar->addWidget(dialShell, 0);
    // Attach the header controls into the scroll area.
    headerScroll->setWidget(headerInner);

    // Row of KPI cards below the header.
    QHBoxLayout* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(10);
    kpiRow->setContentsMargins(14, 0, 14, 0);
    kpiRow->addWidget(makeKpiCard(processedLabel), 1);
    kpiRow->addWidget(makeKpiCard(failedLabel), 1);
    kpiRow->addWidget(makeKpiCard(queueLabel), 1);
    kpiRow->addWidget(makeKpiCard(workersLabel), 1);
    kpiRow->addWidget(makeKpiCard(compressionLabel), 1);

    // Upper layout holds the header, KPI row, progress, and charts.
    QVBoxLayout* upperLay = new QVBoxLayout();
    upperLay->setSpacing(10);
    upperLay->setContentsMargins(0, 10, 0, 10);
    upperLay->addWidget(topStrip);
    upperLay->addWidget(headerScroll);
    upperLay->addLayout(kpiRow);
    upperLay->addWidget(progressBar);

    // Horizontal splitter for the three charts (compression/queue/failure).
    QSplitter* chartSplit = new QSplitter(Qt::Horizontal);
    chartSplit->setChildrenCollapsible(false);
    chartSplit->addWidget(compressionChart);
    chartSplit->addWidget(queueChart);
    chartSplit->addWidget(failureChart);
    chartSplit->setStretchFactor(0, 1);
    chartSplit->setStretchFactor(1, 1);
    chartSplit->setStretchFactor(2, 1);
    chartSplit->setHandleWidth(5);
    upperLay->addWidget(chartSplit, 1);

    // Dock widget wrapping the upper layout so it can live inside the main vertical splitter.
    QWidget* upperDock = new QWidget();
    upperDock->setLayout(upperLay);
    upperDock->setMinimumHeight(300);
    upperDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Main vertical splitter: upper dashboard, table, and logs.
    QSplitter* mainSplit = new QSplitter(Qt::Vertical);
    mainSplit->setObjectName(QStringLiteral("mainSplit"));
    mainSplit->setChildrenCollapsible(false);
    mainSplit->setHandleWidth(6);
    mainSplit->addWidget(upperDock);
    mainSplit->addWidget(requestsTable);
    mainSplit->addWidget(logBox);
    mainSplit->setStretchFactor(0, 1);
    mainSplit->setStretchFactor(1, 3);
    mainSplit->setStretchFactor(2, 1);
    mainSplit->setSizes(QList<int>() << 360 << 420 << 140);

    // Insert the main splitter into the outer layout.
    outer->addWidget(mainSplit, 1);

    // Finalize main window wiring: set central widget, title, and stylesheet.
    setCentralWidget(central);
    setWindowTitle(QStringLiteral("Distributed File Backup Manager — Industrial SCADA"));
    setStyleSheet(industrialStylesheet());

    // Connect UI signals to slots that start/stop and adjust speed/filtering.
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(speedDial, &QDial::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    connect(sourceFilter, &QComboBox::currentTextChanged, this, &MainWindow::onFilterChanged);

    // Route BackupManager logs to the UI thread using QueuedConnection.
    manager.setLogCallback([this](const std::string& msg) {
        QMetaObject::invokeMethod(
            this,
            [this, msg]() {
                log(QString::fromStdString(msg));
            },
            Qt::QueuedConnection);
    });

    // Route progress updates to progress bar + compression chart + KPI label.
    manager.setProgressCallback([this](int value) {
        QMetaObject::invokeMethod(
            this,
            [this, value]() {
                progressBar->setValue(value);
                compressionChart->addPoint(value);
                setKpiRichText(compressionLabel, QStringLiteral("Compression"),
                               QString::number(value) + QStringLiteral("%"));
            },
            Qt::QueuedConnection);
    });

    // Route stats updates to KPI labels and derived charts (queue pressure, failure rate).
    manager.setStatsCallback([this](const BackupManager::Stats& stats) {
        QMetaObject::invokeMethod(
            this,
            [this, stats]() {
                setKpiRichText(processedLabel, QStringLiteral("Total requests"),
                               QString::number(static_cast<qulonglong>(stats.processed)));
                setKpiRichText(failedLabel, QStringLiteral("Failures"),
                               QString::number(static_cast<qulonglong>(stats.failed)));
                setKpiRichText(queueLabel, QStringLiteral("Queue depth"),
                               QString::number(static_cast<qulonglong>(stats.queueDepth)));
                setKpiRichText(workersLabel, QStringLiteral("Active workers"), QString::number(stats.activeWorkers));
                queueChart->addPoint(static_cast<int>(stats.queueDepth > 100 ? 100 : stats.queueDepth));
                int failurePressure = stats.processed > 0
                    ? static_cast<int>((100.0 * static_cast<double>(stats.failed)) / static_cast<double>(stats.processed))
                    : 0;
                failureChart->addPoint(failurePressure);
            },
            Qt::QueuedConnection);
    });
    // Route per-request events to the table, then apply current filters.
    manager.setRequestCallback([this](const BackupManager::RequestEvent& event) {
        QMetaObject::invokeMethod(
            this,
            [this, event]() {
                addRequestRow(event);
                applyTableFilter();
            },
            Qt::QueuedConnection);
    });

    // Tell the simulator how to dispatch generated requests into the manager.
    simulator.setDispatcher([this](const BackupRequest& req) {
        manager.submit(req);
    });
    // Initialize simulator pacing based on the default dial value.
    onSpeedChanged(speedDial->value());
}

MainWindow::~MainWindow() {
    // Ensure background activity is stopped before destroying UI-owned objects.
    onStopClicked();
    // Delete dynamically allocated Client objects created in onStartClicked.
    for (std::size_t i = 0; i < clients.size(); ++i) {
        delete clients[i];
    }
    // Clear the vector after deleting to avoid dangling pointers.
    clients.clear();
}

void MainWindow::log(const QString& msg) {
    // Append a new line of text to the log output widget.
    logBox->append(msg);
}

void MainWindow::onStartClicked() {
    // Prevent starting twice; log state and return.
    if (started) {
        log(QStringLiteral("[STATE] System already running."));
        return;
    }

    // Mark system as started and reset UI components for a new run.
    started = true;
    progressBar->setValue(0);
    requestsTable->setRowCount(0);
    log(QStringLiteral("[STATE] Backup system started."));

    // Lazily create clients only once and register them with the simulator.
    if (clients.empty()) {
        for (int i = 0; i < 10; ++i) {
            Client* c = new Client(i + 1, "Client_" + std::to_string(i + 1));
            clients.push_back(c);
            simulator.addClient(c);
        }
    }

    // Start worker threads and start the client simulator producing requests.
    manager.start();
    simulator.start();
}

void MainWindow::onStopClicked() {
    // If already stopped, there's nothing to do.
    if (!started) {
        return;
    }

    // Mark system as stopped and stop/flush both simulator and manager.
    started = false;
    simulator.stop();
    simulator.wait();
    manager.stop();
    manager.wait();
    // Log a final state transition message.
    log(QStringLiteral("[STATE] Backup system stopped."));
}

void MainWindow::onSpeedChanged(int value) {
    // Update the on-screen label that mirrors the dial value.
    speedLabel->setText(QStringLiteral("THROUGHPUT — %1/10").arg(value));
    // Translate dial value into a pacing window (higher value -> smaller delays).
    int minMs = 80 + (10 - value) * 40;
    int maxMs = minMs + 250;
    // Apply pacing range to the simulator so request generation speed changes.
    simulator.setPacing(minMs, maxMs);
}

void MainWindow::addRequestRow(const BackupManager::RequestEvent& event) {
    // Append a new row at the bottom of the table.
    int row = requestsTable->rowCount();
    requestsTable->insertRow(row);
    // Column 0: sequential row id (1-based).
    requestsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    // Column 1: client id.
    requestsTable->setItem(row, 1, new QTableWidgetItem(QString::number(event.clientId)));
    // Column 2: file name.
    requestsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(event.fileName)));
    // Column 3: file size.
    requestsTable->setItem(row, 3, new QTableWidgetItem(QString::number(static_cast<qulonglong>(event.fileSize))));
    // Column 4: stored bytes (compressed bytes or 0 if skipped).
    requestsTable->setItem(row, 4, new QTableWidgetItem(QString::number(static_cast<qulonglong>(event.storedBytes))));
    // Column 5: source label.
    requestsTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(event.source)));
    // Column 6: status text with color coding.
    QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(event.status));
    // Cache status as QString for case-insensitive checks.
    const QString st = QString::fromStdString(event.status);
    // Choose a color based on the status category.
    if (st.contains(QStringLiteral("Confirmed"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#6ee7b7"))));
    } else if (st.contains(QStringLiteral("Checked In"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#fcd34d"))));
    } else if (st.contains(QStringLiteral("Due In"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#7eb8da"))));
    } else {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#9aa0a8"))));
    }
    // Install the colored status item into the table.
    requestsTable->setItem(row, 6, statusItem);
    // Column 7: compression percentage text.
    requestsTable->setItem(row, 7, new QTableWidgetItem(QString::number(event.compressionPct) + QStringLiteral("%")));
    // Column 8: current local time of UI insertion (for display).
    requestsTable->setItem(row, 8, new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"))));
    // Keep the newest row visible.
    requestsTable->scrollToBottom();
}

void MainWindow::applyTableFilter() {
    // Capture current filter selections.
    QString source = sourceFilter->currentText();
    QString query = searchInput->text().trimmed();
    // Walk all rows and hide any that do not match the source/query predicates.
    for (int r = 0; r < requestsTable->rowCount(); ++r) {
        // Read relevant columns for filtering.
        QString file = requestsTable->item(r, 2)->text();
        QString client = requestsTable->item(r, 1)->text();
        QString rowSource = requestsTable->item(r, 5)->text();
        // Source matches if "All" is selected or the row source equals selected source.
        bool sourceOk = (source == QStringLiteral("All Sources")) || (rowSource == source);
        // Query matches if empty, or is contained in file/client text (case-insensitive).
        bool queryOk = query.isEmpty() || file.contains(query, Qt::CaseInsensitive)
            || client.contains(query, Qt::CaseInsensitive);
        // Hide row if either predicate fails.
        requestsTable->setRowHidden(r, !(sourceOk && queryOk));
    }
}

void MainWindow::onFilterChanged() {
    // Recompute visibility for all rows whenever filter UI changes.
    applyTableFilter();
}
