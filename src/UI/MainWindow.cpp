#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QWidget>
#include <QMetaObject>
#include <QString>
#include <QHeaderView>
#include <QDateTime>
#include <QSplitter>
#include <QScrollArea>
#include <QSizePolicy>
#include <QAbstractItemView>
#include <QBrush>
#include <QFont>

namespace {

void setKpiRichText(QLabel* label, const QString& title, const QString& value) {
    label->setTextFormat(Qt::RichText);
    label->setText(
        QStringLiteral("<div style=\"color:#7a8088;font-size:10px;letter-spacing:2px;text-transform:uppercase;\">%1</div>"
                       "<div style=\"color:#e8eaed;font-size:22px;font-weight:600;font-family:'Consolas','Courier New',monospace;margin-top:6px;\">%2</div>")
            .arg(title, value));
}

QString industrialStylesheet() {
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
    QFrame* card = new QFrame();
    card->setObjectName(QStringLiteral("kpiCard"));
    QVBoxLayout* c = new QVBoxLayout(card);
    c->setContentsMargins(14, 12, 14, 14);
    c->addWidget(label);
    return card;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), manager(4, 2), started(false) {

    QWidget* central = new QWidget(this);
    central->setObjectName(QStringLiteral("centralRoot"));
    QVBoxLayout* outer = new QVBoxLayout(central);
    outer->setSpacing(0);
    outer->setContentsMargins(0, 0, 0, 0);

    startBtn = new QPushButton(QStringLiteral("START"));
    startBtn->setObjectName(QStringLiteral("startBtn"));
    stopBtn = new QPushButton(QStringLiteral("STOP"));
    stopBtn->setObjectName(QStringLiteral("stopBtn"));
    logBox = new QTextEdit();
    progressBar = new QProgressBar();
    speedDial = new QDial();
    speedLabel = new QLabel(QStringLiteral("THROUGHPUT"));
    speedLabel->setObjectName(QStringLiteral("subtitle"));
    processedLabel = new QLabel();
    failedLabel = new QLabel();
    queueLabel = new QLabel();
    workersLabel = new QLabel();
    compressionLabel = new QLabel();
    searchInput = new QLineEdit();
    sourceFilter = new QComboBox();
    requestsTable = new QTableWidget();
    compressionChart = new LineChartWidget();
    queueChart = new LineChartWidget();
    failureChart = new LineChartWidget();

    setKpiRichText(processedLabel, QStringLiteral("Total requests"), QStringLiteral("0"));
    setKpiRichText(failedLabel, QStringLiteral("Failures"), QStringLiteral("0"));
    setKpiRichText(queueLabel, QStringLiteral("Queue depth"), QStringLiteral("0"));
    setKpiRichText(workersLabel, QStringLiteral("Active workers"), QStringLiteral("0"));
    setKpiRichText(compressionLabel, QStringLiteral("Compression"), QStringLiteral("0%"));

    logBox->setReadOnly(true);
    logBox->setMinimumHeight(96);
    logBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat(QStringLiteral("%p%"));
    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    speedDial->setRange(1, 10);
    speedDial->setValue(5);
    speedDial->setNotchesVisible(true);
    speedDial->setFixedSize(72, 72);
    speedDial->setWrapping(false);

    searchInput->setPlaceholderText(QStringLiteral("Filter by file or client ID…"));
    searchInput->setMinimumWidth(200);
    sourceFilter->addItems(QStringList() << QStringLiteral("All Sources")
                                          << QStringLiteral("Booking.com") << QStringLiteral("Expedia")
                                          << QStringLiteral("airbnb"));

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

    compressionChart->setTitle(QStringLiteral("Compression ratio"));
    compressionChart->setLineColor(QColor(QStringLiteral("#c9a227")));
    queueChart->setTitle(QStringLiteral("Queue pressure"));
    queueChart->setLineColor(QColor(QStringLiteral("#7eb8da")));
    failureChart->setTitle(QStringLiteral("Failure rate"));
    failureChart->setLineColor(QColor(QStringLiteral("#c97a5a")));

    QFrame* topStrip = new QFrame();
    topStrip->setObjectName(QStringLiteral("topStrip"));
    QHBoxLayout* topStripLay = new QHBoxLayout(topStrip);
    topStripLay->setContentsMargins(14, 8, 14, 8);

    QLabel* title = new QLabel(QStringLiteral("BACKUP CONTROL — OPERATIONS"));
    title->setObjectName(QStringLiteral("titleLabel"));
    QLabel* ver = new QLabel(QStringLiteral("DFBMS v1"));
    ver->setObjectName(QStringLiteral("subtitle"));
    topStripLay->addWidget(title);
    topStripLay->addStretch();
    topStripLay->addWidget(ver);

    QScrollArea* headerScroll = new QScrollArea();
    headerScroll->setWidgetResizable(true);
    headerScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    headerScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    headerScroll->setFrameShape(QFrame::NoFrame);
    headerScroll->setMaximumHeight(100);
    headerScroll->setMinimumHeight(72);

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
    QFrame* dialShell = new QFrame();
    dialShell->setObjectName(QStringLiteral("dialShell"));
    QVBoxLayout* dialLay = new QVBoxLayout(dialShell);
    dialLay->setContentsMargins(8, 6, 8, 6);
    dialLay->addWidget(speedDial, 0, Qt::AlignCenter);
    headerBar->addWidget(dialShell, 0);
    headerScroll->setWidget(headerInner);

    QHBoxLayout* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(10);
    kpiRow->setContentsMargins(14, 0, 14, 0);
    kpiRow->addWidget(makeKpiCard(processedLabel), 1);
    kpiRow->addWidget(makeKpiCard(failedLabel), 1);
    kpiRow->addWidget(makeKpiCard(queueLabel), 1);
    kpiRow->addWidget(makeKpiCard(workersLabel), 1);
    kpiRow->addWidget(makeKpiCard(compressionLabel), 1);

    QVBoxLayout* upperLay = new QVBoxLayout();
    upperLay->setSpacing(10);
    upperLay->setContentsMargins(0, 10, 0, 10);
    upperLay->addWidget(topStrip);
    upperLay->addWidget(headerScroll);
    upperLay->addLayout(kpiRow);
    upperLay->addWidget(progressBar);

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

    QWidget* upperDock = new QWidget();
    upperDock->setLayout(upperLay);
    upperDock->setMinimumHeight(300);
    upperDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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

    outer->addWidget(mainSplit, 1);

    setCentralWidget(central);
    setWindowTitle(QStringLiteral("Distributed File Backup Manager — Industrial SCADA"));
    setStyleSheet(industrialStylesheet());

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(speedDial, &QDial::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    connect(sourceFilter, &QComboBox::currentTextChanged, this, &MainWindow::onFilterChanged);

    manager.setLogCallback([this](const std::string& msg) {
        QMetaObject::invokeMethod(
            this,
            [this, msg]() {
                log(QString::fromStdString(msg));
            },
            Qt::QueuedConnection);
    });

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
    manager.setRequestCallback([this](const BackupManager::RequestEvent& event) {
        QMetaObject::invokeMethod(
            this,
            [this, event]() {
                addRequestRow(event);
                applyTableFilter();
            },
            Qt::QueuedConnection);
    });

    simulator.setDispatcher([this](const BackupRequest& req) {
        manager.submit(req);
    });
    onSpeedChanged(speedDial->value());
}

MainWindow::~MainWindow() {
    onStopClicked();
    for (std::size_t i = 0; i < clients.size(); ++i) {
        delete clients[i];
    }
    clients.clear();
}

void MainWindow::log(const QString& msg) {
    logBox->append(msg);
}

void MainWindow::onStartClicked() {
    if (started) {
        log(QStringLiteral("[STATE] System already running."));
        return;
    }

    started = true;
    progressBar->setValue(0);
    requestsTable->setRowCount(0);
    log(QStringLiteral("[STATE] Backup system started."));

    if (clients.empty()) {
        for (int i = 0; i < 10; ++i) {
            Client* c = new Client(i + 1, "Client_" + std::to_string(i + 1));
            clients.push_back(c);
            simulator.addClient(c);
        }
    }

    manager.start();
    simulator.start();
}

void MainWindow::onStopClicked() {
    if (!started) {
        return;
    }

    started = false;
    simulator.stop();
    simulator.wait();
    manager.stop();
    manager.wait();
    log(QStringLiteral("[STATE] Backup system stopped."));
}

void MainWindow::onSpeedChanged(int value) {
    speedLabel->setText(QStringLiteral("THROUGHPUT — %1/10").arg(value));
    int minMs = 80 + (10 - value) * 40;
    int maxMs = minMs + 250;
    simulator.setPacing(minMs, maxMs);
}

void MainWindow::addRequestRow(const BackupManager::RequestEvent& event) {
    int row = requestsTable->rowCount();
    requestsTable->insertRow(row);
    requestsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    requestsTable->setItem(row, 1, new QTableWidgetItem(QString::number(event.clientId)));
    requestsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(event.fileName)));
    requestsTable->setItem(row, 3, new QTableWidgetItem(QString::number(static_cast<qulonglong>(event.fileSize))));
    requestsTable->setItem(row, 4, new QTableWidgetItem(QString::number(static_cast<qulonglong>(event.storedBytes))));
    requestsTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(event.source)));
    QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(event.status));
    const QString st = QString::fromStdString(event.status);
    if (st.contains(QStringLiteral("Confirmed"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#6ee7b7"))));
    } else if (st.contains(QStringLiteral("Checked In"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#fcd34d"))));
    } else if (st.contains(QStringLiteral("Due In"), Qt::CaseInsensitive)) {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#7eb8da"))));
    } else {
        statusItem->setForeground(QBrush(QColor(QStringLiteral("#9aa0a8"))));
    }
    requestsTable->setItem(row, 6, statusItem);
    requestsTable->setItem(row, 7, new QTableWidgetItem(QString::number(event.compressionPct) + QStringLiteral("%")));
    requestsTable->setItem(row, 8, new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"))));
    requestsTable->scrollToBottom();
}

void MainWindow::applyTableFilter() {
    QString source = sourceFilter->currentText();
    QString query = searchInput->text().trimmed();
    for (int r = 0; r < requestsTable->rowCount(); ++r) {
        QString file = requestsTable->item(r, 2)->text();
        QString client = requestsTable->item(r, 1)->text();
        QString rowSource = requestsTable->item(r, 5)->text();
        bool sourceOk = (source == QStringLiteral("All Sources")) || (rowSource == source);
        bool queryOk = query.isEmpty() || file.contains(query, Qt::CaseInsensitive)
            || client.contains(query, Qt::CaseInsensitive);
        requestsTable->setRowHidden(r, !(sourceOk && queryOk));
    }
}

void MainWindow::onFilterChanged() {
    applyTableFilter();
}
