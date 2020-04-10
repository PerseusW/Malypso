#include "stock.h"

RealtimeRecord *RealtimeRecord::fromString(const QString& line)
{
    QStringList list = line.split(',');
    if (list.size() < 32) {return nullptr;}
    RealtimeRecord *record = new RealtimeRecord;
    record->code = list.at(0).split('=').first().right(6).toDouble();
    record->openPrice = list.at(1).toDouble();
    record->previousClosePrice = list.at(2).toDouble();
    record->highPrice = list.at(4).toDouble();
    record->lowPrice = list.at(5).toDouble();
    record->exchangedStock = list.at(8).toDouble();
    record->exchangedCurrency = list.at(9).toDouble();
    for (int i = 10; i < 20; i += 2) {
        record->buyPrice.insert(list.at(i + 1).toDouble(), list.at(i).toDouble());
    }
    for (int i = 20; i < 30; i += 2) {
        record->sellPrice.insert(list.at(i + 1).toDouble(), list.at(i).toDouble());
    }
    record->time = list.at(30) + " " + list.at(31);
    return record;
}

QVector<QString> RealtimeRecord::getBasicInfo() const
{
    QVector<QString> basicInfo;
    basicInfo.append(QString::number(code));
    basicInfo.append(QString::number(openPrice));
    basicInfo.append(QString::number(previousClosePrice));
    basicInfo.append(QString::number(highPrice));
    basicInfo.append(QString::number(lowPrice));
    if (exchangedStock > 1000000) {basicInfo.append(QString::number(exchangedStock/1000000)+" M");}
    else if (exchangedStock > 1000) {basicInfo.append(QString::number(exchangedStock/1000)+" K");}
    else {basicInfo.append(QString::number(exchangedStock));}
    if (exchangedCurrency > 1000000) {basicInfo.append(QString::number(exchangedCurrency/1000000)+" M");}
    else if (exchangedCurrency > 1000) {basicInfo.append(QString::number(exchangedCurrency/1000)+" K");}
    else {basicInfo.append(QString::number(exchangedCurrency));}
    basicInfo.append(time);
    return basicInfo;
}


/*****/


DataManager::DataManager(QObject *parent)
    :QObject(parent)
{
    if (file->open(QFile::ReadOnly)) {
        QTextStream inputStream(file);
        while (!inputStream.atEnd()) {
            stocks.insert(inputStream.readLine(),0);
        }
        file->close();
    }
    for (int i = 0; i < 4; ++i) {
        kLines.append(new QtCharts::QCandlestickSeries);
        kLines.last()->setIncreasingColor(QColor(Qt::red));
        kLines.last()->setDecreasingColor(QColor(Qt::green));
    }
    connect(manager,&QNetworkAccessManager::finished,this,&DataManager::readData);
    connect(this,&DataManager::taskFinished,this,&DataManager::iterateTasks);
}

DataManager::~DataManager()
{
    if (file->open(QFile::WriteOnly|QFile::Truncate)) {
        QTextStream outputStream(file);
        for (QString& stock: stocks.keys()) {
            outputStream << stock << endl;
        }
        file->close();
    }
}

void DataManager::requestRealtimeData()
{
    QString url = REAL_TIME; url.replace('*',focus);
    manager->get(QNetworkRequest(QUrl(url)));
}

void DataManager::requestKLineData()
{
    QString url = HISTORY;
    url.replace("*1",focus);
    switch (currentTask) {
    case 1: {url.replace("*2","5"); break;}
    case 2: {url.replace("*2","15"); break;}
    case 3: {url.replace("*2","30"); break;}
    case 4: {url.replace("*2","60"); break;}
    }
    manager->get(QNetworkRequest(QUrl(url)));
}

void DataManager::updateFocus(const QString &stock)
{
    emit syncronizeKLines(kLines);
    this->focus = stock;
    currentTask = 0;
    requestRealtimeData();
}

void DataManager::readData(QNetworkReply *reply)
{
    reply->deleteLater();
    QString data = reply->readAll();
    switch (currentTask) {
    case 0: {
        record = RealtimeRecord::fromString(data);
        if (record) {failCount = 0;}
        else {++failCount;}
        emit taskFinished();
        break;
    }
    default: {
        QStringList tuples = data.split("},{");
        kLines.at(currentTask - 1)->clear();
        for (QString& tuple: tuples) {
            QStringList cells = tuple.remove('\"').split(',');
            if (cells.size() < 5) {
                kLines.at(currentTask - 1)->clear();
                ++failCount;
                emit taskFinished();
                return;
            }
            QtCharts::QCandlestickSet *candleStick = new QtCharts::QCandlestickSet();
            candleStick->setTimestamp(QDateTime::fromString(cells.at(0).right(19),"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch());
            candleStick->setOpen(cells.at(1).split(':').last().toDouble());
            candleStick->setHigh(cells.at(2).split(':').last().toDouble());
            candleStick->setLow(cells.at(3).split(':').last().toDouble());
            candleStick->setClose(cells.at(4).split(':').last().toDouble());
            kLines.at(currentTask - 1)->append(candleStick);
        }
        kLines.at(currentTask - 1)->setName(focus);
        failCount = 0;
        emit taskFinished();
        break;
    }
    }
}

void DataManager::iterateTasks()
{
    if (currentTask < 0 || currentTask > 4) {
        currentTask = -1;
    }
    else if (failCount == 0) {
        if (currentTask == 0) {
            emit updateRealtimeDataView(record);
            ++currentTask;
            requestKLineData();
        }
        else if (currentTask == 4) {
            emit updateChartView(currentTask - 1);
            stocks.insert(focus,QDateTime::currentSecsSinceEpoch());
            currentTask = -1;
        }
        else {
            emit updateChartView(currentTask - 1);
            ++currentTask;
            requestKLineData();
        }
    }
    else if (failCount < 3) {
        if (currentTask == 0) {emit(throwError(0));}
        else {requestKLineData();}
    }
    else {
        failCount = 0;
    }
}


/*****/


QueryPad::QueryPad(QWidget *parent)
    :QGroupBox(parent)
{
    this->setTitle("Query Pad");
    QGridLayout *layout = new QGridLayout(this);
    QPushButton *button = new QPushButton("Query",this);
    layout->addWidget(lineEdit,1,0,1,4);
    layout->addWidget(button,2,0,1,4);
    connect(button,&QPushButton::clicked,this,&QueryPad::emitQuerySignal);
}

void QueryPad::emitQuerySignal()
{
    QString stock = lineEdit->text();
    if (stock.startsWith("6")) {stock.prepend("sh");}
    else if (stock.startsWith("0")) {stock.prepend("sz");}
    emit(queryStock(stock));
}


/*****/


InfoPad::InfoPad(QWidget *parent)
    :QGroupBox(parent)
{
    this->setTitle("Info Pad");
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    for (int i = 0; i < 8; ++i) {
        QLabel *label = new QLabel("*******************",this);
        label->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
        basicInfoLabels.append(label);
    }
    mainLayout->addWidget(new QLabel("Stock Code:",this),0,0);
    mainLayout->addWidget(basicInfoLabels.at(0),0,1);
    mainLayout->addWidget(new QLabel("Open Price:",this),1,0);
    mainLayout->addWidget(basicInfoLabels.at(1),1,1);
    mainLayout->addWidget(new QLabel("Previous Close Price:",this),2,0);
    mainLayout->addWidget(basicInfoLabels.at(2),2,1);
    mainLayout->addWidget(new QLabel("Highest Price:",this),3,0);
    mainLayout->addWidget(basicInfoLabels.at(3),3,1);
    mainLayout->addWidget(new QLabel("Lowest Price:",this),4,0);
    mainLayout->addWidget(basicInfoLabels.at(4),4,1);
    mainLayout->addWidget(new QLabel("Exchanged Stock:",this),5,0);
    mainLayout->addWidget(basicInfoLabels.at(5),5,1);
    mainLayout->addWidget(new QLabel("Exchanged Currency:",this),6,0);
    mainLayout->addWidget(basicInfoLabels.at(6),6,1);
    mainLayout->addWidget(new QLabel("Time:",this),7,0);
    mainLayout->addWidget(basicInfoLabels.at(7),7,1);
    QGridLayout *bottomLayout = new QGridLayout();
    mainLayout->addLayout(bottomLayout,8,0,1,2,Qt::AlignTop);
    bottomLayout->addWidget(new QLabel("Buy",this),0,0,1,2,Qt::AlignCenter);
    bottomLayout->addWidget(new QLabel("Sell",this),0,2,1,2,Qt::AlignCenter);
    for (int i = 0; i < 5; ++i) {
        buyPriceLabels.append(new QLabel(QString::number(i),this));
        buyAmountLabels.append(new QLabel(QString::number(i),this));
        sellPriceLabels.append(new QLabel(QString::number(i),this));
        sellAmountLabels.append(new QLabel(QString::number(i),this));
        bottomLayout->addWidget(buyPriceLabels.last(),i + 1,0,Qt::AlignLeft);
        bottomLayout->addWidget(buyAmountLabels.last(),i + 1,1,Qt::AlignCenter);
        bottomLayout->addWidget(sellAmountLabels.last(),i + 1,2,Qt::AlignCenter);
        bottomLayout->addWidget(sellPriceLabels.last(),i + 1,3,Qt::AlignRight);
    }

}

void InfoPad::updateStockFocus(const RealtimeRecord* record)
{
    QVector<QString> tuples = record->getBasicInfo();
    for (int i = 0; i < basicInfoLabels.size(); ++i) {
        basicInfoLabels.at(i)->setText(tuples.at(i));
    }
    QList<double> buyPrices = record->buyPrice.keys();
    QList<double> buyAmounts = record->buyPrice.values();
    QList<double> sellPrices = record->sellPrice.keys();
    QList<double> sellAmounts = record->sellPrice.values();
    QString price;
    for (int i = 0; i < 5; ++i) {
        buyPriceLabels.at(i)->setText(price.setNum(buyPrices.at(4 - i),'f',2));
        buyAmountLabels.at(i)->setNum(qRound(buyAmounts.at(4 - i) / 100));
        sellPriceLabels.at(i)->setText(price.setNum(sellPrices.at(i),'f',2));
        sellAmountLabels.at(i)->setNum(qRound(sellAmounts.at(i) / 100));
    }
}


/*****/


StockCharts::StockCharts(QWidget *parent)
    :QTabWidget(parent)
{
    for (int i = 0; i < 4; ++i) {
        charts.append(new QtCharts::QChart());
        charts.last()->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
        QtCharts::QChartView *chartView = new QtCharts::QChartView(charts.last(),this);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setRubberBand(QtCharts::QChartView::HorizontalRubberBand);
        switch (i) {
        case 0: {this->addTab(chartView,"5 Min"); break;}
        case 1: {this->addTab(chartView,"15 Min"); break;}
        case 2: {this->addTab(chartView,"30 Min"); break;}
        case 3: {this->addTab(chartView,"60 Min"); break;}
        }
    }
}

void StockCharts::syncronizeKLines(const QVector<QtCharts::QCandlestickSeries*>& kLines)
{
    this->kLines = kLines;
}

void StockCharts::updateChartView(const int& index)
{
    if (!charts.at(index)->series().empty()) {
        charts.at(index)->removeSeries(kLines.at(index));
    }
    charts.at(index)->addSeries(kLines.at(index));
    charts.at(index)->createDefaultAxes();
}


/*****/


StockWidget::StockWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setLayout(getMainLayout());
    connect(queryPad,&QueryPad::queryStock,dataManager,&DataManager::updateFocus);
    connect(dataManager,&DataManager::throwError,this,&StockWidget::showError);
    connect(dataManager,&DataManager::updateRealtimeDataView,infoPad,&InfoPad::updateStockFocus);
    connect(dataManager,&DataManager::syncronizeKLines,stockCharts,&StockCharts::syncronizeKLines);
    connect(dataManager,&DataManager::updateChartView,stockCharts,&StockCharts::updateChartView);
}

QHBoxLayout *StockWidget::getMainLayout()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(stockCharts,19);
    layout->addLayout(getRightLayout(),1);
    return layout;
}

QVBoxLayout *StockWidget::getRightLayout()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(infoPad,0,Qt::AlignTop);
    layout->addWidget(queryPad,0,Qt::AlignBottom);
    return layout;
}

void StockWidget::showError(const int&)
{
    QMessageBox::warning(this,"Error","Stock doesn't exist");
}
