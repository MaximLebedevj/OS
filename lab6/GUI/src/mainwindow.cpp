#include "mainwindow.h"

class HourScaleDraw : public QwtScaleDraw {
public:
    HourScaleDraw() {}

    virtual QwtText label(double value) const override {
        int hour = static_cast<int>(value);
        return QString::number(hour);
    }
};

class WeekScaleDraw : public QwtScaleDraw {
public:
    WeekScaleDraw() {}

    virtual QwtText label(double value) const override {
        int hour = static_cast<int>(value);
        int day = hour / 24;
        QStringList days = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        QString label = days[day % 7];
        return label;
    }
};

class MonthScaleDraw : public QwtScaleDraw {
public:
    MonthScaleDraw() {}

    virtual QwtText label(double value) const override {
        int hour = static_cast<int>(value);
        int week = hour / (24 * 7);
        return QString::number(week);
    }
};

class DayWeekScaleDraw : public QwtScaleDraw {
public:
    explicit DayWeekScaleDraw(const QVector<QDate> &dates) : dates_(dates) {}

    virtual QwtText label(double value) const override {
        int index = static_cast<int>(value);
        if (index >= 0 && index < dates_.size()) {
            return dates_[index].toString("MM-dd");
        }
        return QString::number(value);
    }

private:
    QVector<QDate> dates_;
};

class DayYearScaleDraw : public QwtScaleDraw {
public:
    explicit DayYearScaleDraw(const QVector<QDate> &dates) : dates_(dates) {}

    virtual QwtText label(double value) const override {
        int index = static_cast<int>(value);
        if (index >= 0 && index < dates_.size()) {
            return dates_[index].toString("MM");
        }
        return QString::number(value);
    }

private:
    QVector<QDate> dates_;
};

class CurrentMinuteDraw : public QwtScaleDraw {
public:
    explicit CurrentMinuteDraw() : QwtScaleDraw() {}

    virtual QwtText label(double value) const override {
        int seconds = static_cast<int>(value);

        int minutes = seconds / 60;
        int remainingSeconds = seconds % 60;

        QString timeLabel = QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(remainingSeconds, 2, 10, QChar('0'));

        return QwtText(timeLabel);
    }
};

MainWindow::MainWindow(QWidget *parent)
: QWidget(parent),
networkManager(new QNetworkAccessManager(this)),
plot(new QwtPlot(this))
{
    temperatureLabel = new QLabel("Current Temperature: --", this);

    temperatureLabel->setStyleSheet("QLabel {"
    "font-family: 'Arial', sans-serif;"
    "font-size: 18px;"
    "color: #333333;"
    "background-color: #f0f0f0;"
    "border: 2px solid #4CAF50;"
    "border-radius: 10px;"
    "padding: 10px;"
    "text-align: center;"
    "}");

    currentMinuteButton = new QPushButton("Current", this);
    connect(currentMinuteButton, &QPushButton::clicked, this, &MainWindow::onCurrentMinuteButtonRequest);

    graphButton = new QPushButton("Hourly - Day", this);
    connect(graphButton, &QPushButton::clicked, this, &MainWindow::onGraphRequest);

    weekGraphButton = new QPushButton("Hourly - Week", this);
    connect(weekGraphButton, &QPushButton::clicked, this, &MainWindow::onWeekGraphRequest);

    monthGraphButton = new QPushButton("Hourly - Month", this);
    connect(monthGraphButton, &QPushButton::clicked, this, &MainWindow::onMonthGraphRequest);

    QFrame *leftFrame = new QFrame(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->addWidget(graphButton);
    leftLayout->addWidget(weekGraphButton);
    leftLayout->addWidget(monthGraphButton);
    leftFrame->setLayout(leftLayout);

    dayWeekGraphButton = new QPushButton("Daily - Week", this);
    connect(dayWeekGraphButton, &QPushButton::clicked, this, &MainWindow::onDailyWeekGraphRequest);

    dayMonthGraphButton = new QPushButton("Daily - Month", this);
    connect(dayMonthGraphButton, &QPushButton::clicked, this, &MainWindow::onDailyMonthGraphRequest);

    dayYearGraphButton = new QPushButton("Daily - Year", this);
    connect(dayYearGraphButton, &QPushButton::clicked, this, &MainWindow::onDailyYearGraphRequest);

    QFrame *rightFrame = new QFrame(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->addWidget(dayWeekGraphButton);
    rightLayout->addWidget(dayMonthGraphButton);
    rightLayout->addWidget(dayYearGraphButton);
    rightFrame->setLayout(rightLayout);

    plot->setTitle("Temperature Graph");
    plot->setCanvasBackground(QColor("#f7f7f7"));
    plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::lightGray, 0, Qt::DotLine));
    grid->attach(plot);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(temperatureLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(currentMinuteButton);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(leftFrame);
    bottomLayout->addWidget(plot);
    bottomLayout->addWidget(rightFrame);

    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRequestData);
    connect(timer, &QTimer::timeout, this, &MainWindow::onCurrentMinuteRequest);
    timer->start(1000);

    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onResponseReceived);

    onRequestData();
    onCurrentMinuteRequest();
}

MainWindow::~MainWindow() {}

void MainWindow::onRequestData()
{
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "current");
    networkManager->get(request);
}

void MainWindow::onGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "hourly_day");
    networkManager->get(request);
}

void MainWindow::onWeekGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "hourly_week");
    networkManager->get(request);
}

void MainWindow::onMonthGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "hourly_month");
    networkManager->get(request);
}

void MainWindow::onDailyWeekGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "daily_week");
    networkManager->get(request);
}

void MainWindow::onDailyMonthGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "daily_month");
    networkManager->get(request);
}

void MainWindow::onDailyYearGraphRequest()
{
    updateCurrentMinute = false;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "daily_year");
    networkManager->get(request);
}

void MainWindow::onCurrentMinuteRequest()
{
    if (!updateCurrentMinute)
        return;

    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "current_minute");
    networkManager->get(request);
}

void MainWindow::onCurrentMinuteButtonRequest()
{
    updateCurrentMinute = true;
    plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    QUrl url("http://127.0.0.1:8080");
    QNetworkRequest request(url);
    request.setRawHeader("X-Client-Type", "qt-app");
    request.setRawHeader("action", "current_minute");
    networkManager->get(request);
}

void MainWindow::onResponseReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        temperatureLabel->setText("Error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);

    if (reply->request().rawHeader("action") == "current") {
        QJsonObject jsonObj = doc.object();
        double currentTemp = jsonObj["current_temp"].toString().toDouble();
        temperatureLabel->setText(QString("Current Temperature: %1 °C").arg(currentTemp));
    }

    QJsonArray data = doc.array();
    if (data.isEmpty()) {
        reply->deleteLater();
        return;
    }

    if (reply->request().rawHeader("action") == "hourly_day") {
        QJsonArray data = doc.array();
        QVector<double> times, temperatures;

        for (const QJsonValue &val : data) {
            QJsonObject entry = val.toObject();
            double temp = entry["TEMP"].toString().toDouble();
            QTime time = QTime::fromString(entry["DATETIME"].toString().mid(11, 5), "HH:mm");
            times.append(time.hour());
            temperatures.append(temp);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(times, temperatures);
        curve->setTitle("Hourly Average Temperature for the Day");
        curve->setPen(QPen(Qt::blue, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::blue), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        plot->setAxisScale(QwtPlot::yLeft,
                           *std::min_element(temperatures.begin(), temperatures.end()) - 10,
                           *std::max_element(temperatures.begin(), temperatures.end()) + 10);

        plot->setAxisScale(QwtPlot::xBottom, 0, 23, 1);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new HourScaleDraw());
        plot->replot();
    }

    if (reply->request().rawHeader("action") == "hourly_week") {
        QJsonArray data = doc.array();
        QVector<double> times, temperatures;

        for (int i = 0; i < data.size(); ++i) {
            QJsonObject entry = data[i].toObject();
            double temp = entry["TEMP"].toString().toDouble();
            times.append(i);
            temperatures.append(temp);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(times, temperatures);
        curve->setTitle("Hourly Average Temperature for the Week");
        curve->setPen(QPen(Qt::green, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(Qt::green), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        plot->setAxisScale(QwtPlot::yLeft,
                           *std::min_element(temperatures.begin(), temperatures.end()) - 10,
                           *std::max_element(temperatures.begin(), temperatures.end()) + 10);

        plot->setAxisScale(QwtPlot::xBottom, 0, 7 * 24, 24);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new WeekScaleDraw());
        plot->replot();
    }

    if (reply->request().rawHeader("action") == "hourly_month") {
        QJsonArray data = doc.array();
        QVector<double> times, temperatures;

        for (int i = 0; i < data.size(); ++i) {
            QJsonObject entry = data[i].toObject();
            double temp = entry["TEMP"].toString().toDouble();
            times.append(i);
            temperatures.append(temp);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(times, temperatures);
        curve->setTitle("Hourly Average Temperature for the Month");
        curve->setPen(QPen(Qt::green, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(Qt::green), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        plot->setAxisScale(QwtPlot::yLeft,
                           *std::min_element(temperatures.begin(), temperatures.end()) - 10,
                           *std::max_element(temperatures.begin(), temperatures.end()) + 10);

        plot->setAxisScale(QwtPlot::xBottom, 0, 30 * 24, 7 * 24);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new MonthScaleDraw());
        plot->replot();
    }

    if (reply->request().rawHeader("action") == "daily_week") {
        QJsonArray data = doc.array();
        QVector<QDate> dates;
        QVector<double> temperatures;

        for (const QJsonValue &value : data) {
            QJsonObject entry = value.toObject();
            QString dateStr = entry["DATE"].toString();
            double temp = entry["TEMP"].toString().toDouble();
            dates.append(QDate::fromString(dateStr, "yyyy-MM-dd"));
            temperatures.append(temp);
        }

        QVector<double> days;
        for (int i = 0; i < dates.size(); ++i) {
            days.append(i);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(days, temperatures);
        curve->setTitle("Daily Average Temperature for the Week");
        curve->setPen(QPen(Qt::blue, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::blue), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        double minTemp = *std::min_element(temperatures.begin(), temperatures.end());
        double maxTemp = *std::max_element(temperatures.begin(), temperatures.end());
        plot->setAxisScale(QwtPlot::yLeft, minTemp - 5, maxTemp + 5);

        plot->setAxisScale(QwtPlot::xBottom, 0, 6);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new DayWeekScaleDraw(dates));

        plot->replot();
    }

    if (reply->request().rawHeader("action") == "daily_month") {
        QJsonArray data = doc.array();
        QVector<QDate> dates;
        QVector<double> temperatures;

        for (const QJsonValue &value : data) {
            QJsonObject entry = value.toObject();
            QString dateStr = entry["DATE"].toString();
            double temp = entry["TEMP"].toString().toDouble();
            dates.append(QDate::fromString(dateStr, "yyyy-MM-dd"));
            temperatures.append(temp);
        }

        QVector<double> days;
        for (int i = 0; i < dates.size(); ++i) {
            days.append(i);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(days, temperatures);
        curve->setTitle("Daily Average Temperature for the Month");
        curve->setPen(QPen(Qt::red, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::red), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        double minTemp = *std::min_element(temperatures.begin(), temperatures.end());
        double maxTemp = *std::max_element(temperatures.begin(), temperatures.end());
        plot->setAxisScale(QwtPlot::yLeft, minTemp - 5, maxTemp + 5);

        plot->setAxisScale(QwtPlot::xBottom, 0, 29);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new DayWeekScaleDraw(dates));

        plot->replot();
    }

    if (reply->request().rawHeader("action") == "daily_year") {
        QJsonArray data = doc.array();
        QVector<QDate> dates;
        QVector<double> temperatures;

        for (const QJsonValue &value : data) {
            QJsonObject entry = value.toObject();
            QString dateStr = entry["DATE"].toString();
            double temp = entry["TEMP"].toString().toDouble();
            dates.append(QDate::fromString(dateStr, "yyyy-MM-dd"));
            temperatures.append(temp);
        }

        QVector<double> days;
        for (int i = 0; i < dates.size(); ++i) {
            days.append(i);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(days, temperatures);
        curve->setTitle("Daily Average Temperature for the Year");
        curve->setPen(QPen(Qt::magenta, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::cyan), QPen(Qt::magenta), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        double minTemp = *std::min_element(temperatures.begin(), temperatures.end());
        double maxTemp = *std::max_element(temperatures.begin(), temperatures.end());
        plot->setAxisScale(QwtPlot::yLeft, minTemp - 5, maxTemp + 5);

        plot->setAxisScale(QwtPlot::xBottom, 0, 365);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new DayYearScaleDraw(dates));

        plot->replot();
    }

    if (reply->request().rawHeader("action") == "current_minute") {
        QJsonArray data = doc.array();
        QVector<double> times, temperatures;

        for (int i = 0; i < data.size(); ++i) {
            QJsonObject entry = data[i].toObject();
            double temp = entry["TEMP"].toString().toDouble();
            times.append(i);
            temperatures.append(temp);
        }

        QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setSamples(times, temperatures);
        curve->setTitle("Current Temperature");
        curve->setPen(QPen(Qt::blue, 3));
        curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::blue), QSize(6, 6)));
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->attach(plot);

        plot->setAxisScale(QwtPlot::yLeft,
                           *std::min_element(temperatures.begin(), temperatures.end()) - 5,
                           *std::max_element(temperatures.begin(), temperatures.end()) + 5);

        plot->setAxisScale(QwtPlot::xBottom, 0, 60, 10);
        plot->setAxisScaleDraw(QwtPlot::xBottom, new CurrentMinuteDraw());
        plot->replot();
    }

    reply->deleteLater();
}
