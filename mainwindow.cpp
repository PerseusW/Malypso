#include "mainwindow.h"

const static int WIN_SIZE = 400;
const static int ICON_SIZE = 400;
const static QMap<QString,int> MODULES = {
    {"Stock",1},
    {"App Manager",2},
    {"Screen Shot",3},
    {"Predictor",4}
};

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
{
    this->setWindowIcon(getIcon());
    this->move(0,0);
    this->setWindowTitle("Malypso");
    CentralWidget *centralWidget = new CentralWidget(this);
    this->setCentralWidget(centralWidget);
    connect(centralWidget,&CentralWidget::createDockWidget,this,&MainWindow::createDockWidget);
    connect(this,&MainWindow::updateWidgetList,centralWidget,&CentralWidget::updateWidgetList);
}

MainWindow::~MainWindow()
{

}

QIcon MainWindow::getIcon()
{
    uint8_t red = 0, green = 255, blue = 0;
    QImage image(ICON_SIZE,ICON_SIZE,QImage::Format_RGB32);
    for (int i = ICON_SIZE / 8; i < ICON_SIZE / 4; ++i) {
        for (int j = ICON_SIZE / 8; j < ICON_SIZE * 7 / 8; ++j) {
            image.setPixel(i,j,qRgb(red,green,blue));
        }
        iterateColor(red,green,blue);
    }
    for (int i = ICON_SIZE / 4; i < ICON_SIZE / 2; ++i) {
        for (int j = i - ICON_SIZE / 8; j < i; ++j) {
            image.setPixel(i,j,qRgb(red,green,blue));
        }
        iterateColor(red,green,blue);
    }
    for (int i = ICON_SIZE / 2; i < ICON_SIZE * 3 / 4; ++i) {
        for (int j = ICON_SIZE * 7 / 8 - i; j < ICON_SIZE - i; ++j) {
            image.setPixel(i,j,qRgb(red,green,blue));
        }
        iterateColor(red,green,blue);
    }
    for (int i = ICON_SIZE * 3 / 4; i < ICON_SIZE * 7 / 8; ++i) {
        for (int j = ICON_SIZE / 8; j < ICON_SIZE * 7 / 8; ++j) {
            image.setPixel(i,j,qRgb(red,green,blue));
        }
        iterateColor(red,green,blue);
    }
    return QIcon(QPixmap::fromImage(image));
}

void MainWindow::iterateColor(uint8_t& red, uint8_t& green, uint8_t& blue)
{
    if (green == 255) {
        if (blue != 0) {--blue;}
        else if (red < 255) {++red;}
        else {--green;}
    }
    else if (red == 255) {
        if (green != 0) {--green;}
        else if (blue < 255) {++blue;}
        else {--red;}
    }
    else if (blue == 255) {
        if (red != 0) {--red;}
        else if (green < 255) {++green;}
        else {--blue;}
    }
}

void MainWindow::createDockWidget(const int &module)
{
    QDockWidget *dockWidget = new QDockWidget(MODULES.key(module),this);
    this->addDockWidget(Qt::RightDockWidgetArea,dockWidget);
    dockWidget->setAttribute(Qt::WA_DeleteOnClose);
    dockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    dockWidget->setFloating(true);
    switch (module) {
    case 1: {
        dockWidget->setWidget(new StockWidget(dockWidget));
        dockWidget->move(this->x() + this->width() + 1,this->y());
        dockWidget->resize(1520,500);
        break;
    }
    case 2: {
        dockWidget->setWidget(new ExternalAppWidget(dockWidget));
        dockWidget->move(this->x(),this->y() + this->height() + 38);
        dockWidget->resize(200,200);
        break;
    }
    case 3: {dockWidget->setWidget(new ScreenShotWidget(dockWidget)); break;}
    }
    this->dockWidgetsChanged();
    connect(dockWidget,&QDockWidget::destroyed,this,&MainWindow::dockWidgetsChanged);
}

void MainWindow::dockWidgetsChanged()
{
    emit(updateWidgetList(this->children()));
}


/*****/


CentralWidget::CentralWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setLayout(getMainLayout());
    this->setFixedSize(WIN_SIZE,WIN_SIZE);
}

QHBoxLayout *CentralWidget::getMainLayout()
{
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(getLeftWidget());
    mainLayout->addLayout(getRightLayout());
    return mainLayout;
}

QGroupBox *CentralWidget::getLeftWidget()
{
    QGroupBox *groupBox = new QGroupBox("Info Pad",this);
    QGridLayout* leftLayout = new QGridLayout(groupBox);
    leftLayout->setAlignment(Qt::AlignTop);
    timeLabel = new QLabel(groupBox);
    leftLayout->addWidget(new QLabel("Time:",groupBox),0,0);
    leftLayout->addWidget(timeLabel,0,1);
    QTimer *timer = new QTimer(groupBox);
    connect(timer,&QTimer::timeout,this,&CentralWidget::updateTime);
    timer->start(100);
    return groupBox;
}

QVBoxLayout *CentralWidget::getRightLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QGroupBox* topGroupBox = new QGroupBox("Widgets Available",this);
    QVBoxLayout *topLayout = new QVBoxLayout(topGroupBox);
    QListWidget *availableList = new QListWidget(topGroupBox);
    connect(availableList,&QListWidget::doubleClicked,this,&CentralWidget::doubleClicked);
    for (QString& name: MODULES.keys()) {availableList->addItem(name);}
    topLayout->addWidget(availableList);
    topGroupBox->setLayout(topLayout);
    mainLayout->addWidget(topGroupBox);
    QGroupBox *bottomGroupBox = new QGroupBox("Widgets Running",this);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomGroupBox);
    runningList = new QListWidget(bottomGroupBox);
    connect(runningList,&QListWidget::doubleClicked,this,&CentralWidget::focusDockWidget);
    bottomLayout->addWidget(runningList);
    bottomGroupBox->setLayout(bottomLayout);
    mainLayout->addWidget(bottomGroupBox);
    return mainLayout;
}

void CentralWidget::updateWidgetList(const QObjectList &objects)
{
    dockWidgets.clear();
    runningList->clear();
    for (QObject* object: objects) {
        QDockWidget* dockWidget = qobject_cast<QDockWidget*>(object);
        if (dockWidget) {
            dockWidgets.append(dockWidget);
            runningList->addItem(dockWidget->windowTitle());
        }
    }
}

void CentralWidget::doubleClicked(const QModelIndex& index)
{
    emit(createDockWidget(MODULES.value(index.data().toString())));
}

void CentralWidget::updateTime()
{
    timeLabel->setText(QTime::currentTime().toString());
}

void CentralWidget::focusDockWidget(const QModelIndex& index)
{
    dockWidgets.at(index.row())->hide();
    dockWidgets.at(index.row())->show();
}
