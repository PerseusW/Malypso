#ifndef STOCK_H
#define STOCK_H

#include <QMap>
#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QVector>
#include <QString>
#include <QLineEdit>
#include <QGroupBox>
#include <QDateTime>
#include <QCheckBox>
#include <QDateTime>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QMessageBox>
#include <QPushButton>
#include <QNetworkReply>
#include <QStackedWidget>
#include <QtCharts/QChartView>
#include <QNetworkAccessManager>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QCandlestickSet>
#include <QtCharts/QCandlestickSeries>
#include <QDebug>

/*****/


class RealtimeRecord
{
public:
    static RealtimeRecord* fromString(const QString&);
    QVector<QString> getBasicInfo() const;
    double code = 0;
    double openPrice = 0;
    double previousClosePrice = 0;
    double highPrice = 0;
    double lowPrice = 0;
    double exchangedStock = 0;
    double exchangedCurrency = 0;
    QMap<double,double> buyPrice = {};
    QMap<double,double> sellPrice = {};
    QString time = "";
};

class DataManager: public QObject
{
    Q_OBJECT
public:
    DataManager(QObject *parent = nullptr);
    ~DataManager();

private:
    //Sina Finance API template
    const QString REAL_TIME = "http://hq.sinajs.cn/list=*";
    const QString HISTORY = "http://money.finance.sina.com.cn/quotes_service/api/json_v2.php/CN_MarketData.getKLineData?symbol=*1&scale=*2&ma=no&datalen=101";

    //HTTP API
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    //Task schedule
    QString focus = "";
    int currentTask = -1;//-1:Vacant,0:Realtime,1:Five,2:Fifteen,3:Thirty,4:Sixty
    int failCount = 0;

    //Data pool
    QFile *file = new QFile("stocks");
    QMap<QString,qint64> stocks = {};
    RealtimeRecord *record = nullptr;
    QVector<QtCharts::QCandlestickSeries*> kLines = {};

    void requestRealtimeData();
    void requestKLineData();

public slots:
    void updateFocus(const QString&);

private slots:
    void readData(QNetworkReply*);
    void iterateTasks();

signals:
    void taskFinished();
    void throwError(const int&);
    void updateRealtimeDataView(const RealtimeRecord*);
    void syncronizeKLines(const QVector<QtCharts::QCandlestickSeries*>&);
    void updateChartView(const int&);
};

/*****/


class QueryPad: public QGroupBox
{
    Q_OBJECT
public:
    QueryPad(QWidget *parent = nullptr);

private:
    QLineEdit *lineEdit = new QLineEdit("688088",this);

private slots:
    void emitQuerySignal();

signals:
    void queryStock(const QString&);
};


/*****/


class InfoPad: public QGroupBox
{
    Q_OBJECT
public:
    InfoPad(QWidget *parent = nullptr);

private:
    QVector<QLabel*> basicInfoLabels;
    QVector<QLabel*> buyPriceLabels;
    QVector<QLabel*> buyAmountLabels;
    QVector<QLabel*> sellPriceLabels;
    QVector<QLabel*> sellAmountLabels;

public slots:
    void updateStockFocus(const RealtimeRecord*);
};


/*****/


class StockCharts: public QTabWidget
{
    Q_OBJECT
public:
    StockCharts(QWidget *parent = nullptr);

private:
    QVector<QtCharts::QChart*> charts;
    QVector<QtCharts::QCandlestickSeries*> kLines;

public slots:
    void syncronizeKLines(const QVector<QtCharts::QCandlestickSeries*>&);
    void updateChartView(const int&);
};


/*****/


class StockWidget: public QWidget
{
    Q_OBJECT
public:
    StockWidget(QWidget *parent = nullptr);

private:
    QHBoxLayout* getMainLayout();
    QVBoxLayout* getRightLayout();
    InfoPad *infoPad = new InfoPad(this);
    QueryPad *queryPad = new QueryPad(this);
    DataManager *dataManager = new DataManager(this);
    StockCharts *stockCharts = new StockCharts(this);

public slots:
    void showError(const int&);
};

#endif // STOCK_H
