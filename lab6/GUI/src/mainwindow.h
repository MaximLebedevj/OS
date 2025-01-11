#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>
#include <qwt_scale_draw.h>
#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRequestData();
    void onGraphRequest();
    void onWeekGraphRequest();
    void onMonthGraphRequest();
    void onDailyWeekGraphRequest();
    void onDailyMonthGraphRequest();
    void onDailyYearGraphRequest();
    void onCurrentMinuteRequest();
    void onCurrentMinuteButtonRequest();
    void onResponseReceived(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QLabel *temperatureLabel;
    QPushButton *graphButton;
    QPushButton *weekGraphButton;
    QPushButton *monthGraphButton;
    QPushButton *dayWeekGraphButton;
    QPushButton *dayMonthGraphButton;
    QPushButton *dayYearGraphButton;
    QPushButton *currentMinuteButton;
    QwtPlot *plot;


    bool updateCurrentMinute = true;
};

#endif // MAINWINDOW_H
