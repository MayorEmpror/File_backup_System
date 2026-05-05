#ifndef LINE_CHART_WIDGET_H
#define LINE_CHART_WIDGET_H

#include <QWidget>
#include <QString>
#include <QColor>
#include <vector>

class LineChartWidget : public QWidget {
    Q_OBJECT

private:
    std::vector<int> values_;
    int maxPoints_;
    QString title_;
    QColor lineColor_;

public:
    explicit LineChartWidget(QWidget* parent = nullptr);
    void addPoint(int value);
    void setTitle(const QString& title);
    void setLineColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif
