#include "LineChartWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QFont>

LineChartWidget::LineChartWidget(QWidget* parent)
    : QWidget(parent), maxPoints_(80), title_("Trend"), lineColor_(QColor("#c9a227")) {
    setMinimumHeight(140);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void LineChartWidget::addPoint(int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > 100) {
        value = 100;
    }

    values_.push_back(value);
    if (static_cast<int>(values_.size()) > maxPoints_) {
        values_.erase(values_.begin());
    }
    update();
}

void LineChartWidget::setTitle(const QString& title) {
    title_ = title;
    update();
}

void LineChartWidget::setLineColor(const QColor& color) {
    lineColor_ = color;
    update();
}

void LineChartWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    const QColor panel("#1e2229");
    const QColor grid("#3d424d");
    const QColor label("#8b929a");
    p.fillRect(rect(), panel);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int left = 34;
    const int right = width() - 12;
    const int top = 14;
    const int bottom = height() - 22;

    p.setPen(QPen(grid, 1));
    for (int i = 0; i <= 4; ++i) {
        int y = top + ((bottom - top) * i) / 4;
        p.drawLine(left, y, right, y);
    }

    p.setPen(QPen(label, 1));
    p.setFont(QFont("Consolas", 8));
    p.drawText(6, top + 10, "100");
    p.drawText(10, bottom + 4, "0");
    p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    p.drawText(left, top + 10, title_);

    if (values_.size() < 2) {
        return;
    }

    p.setPen(QPen(lineColor_, 2));
    const int n = static_cast<int>(values_.size());
    for (int i = 1; i < n; ++i) {
        int x1 = left + ((right - left) * (i - 1)) / (maxPoints_ - 1);
        int x2 = left + ((right - left) * i) / (maxPoints_ - 1);
        int y1 = bottom - ((bottom - top) * values_[i - 1]) / 100;
        int y2 = bottom - ((bottom - top) * values_[i]) / 100;
        p.drawLine(x1, y1, x2, y2);
    }
}
