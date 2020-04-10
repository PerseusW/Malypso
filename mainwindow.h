#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "app.h"
#include "stock.h"
#include "screenshot.h"

#include <QRgb>
#include <QIcon>
#include <QImage>
#include <QTimer>
#include <QMainWindow>
#include <QDockWidget>
#include <QObjectList>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QIcon getIcon();
    void iterateColor(uint8_t&,uint8_t&,uint8_t&);

private slots:
    void createDockWidget(const int&);
    void dockWidgetsChanged();

signals:
    void updateWidgetList(const QObjectList&);
};


/*****/


class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget(QWidget *parent = nullptr);

private:
    QHBoxLayout* getMainLayout();
    QGroupBox* getLeftWidget();
    QVBoxLayout* getRightLayout();
    QLabel* timeLabel;
    QListWidget *runningList;
    QVector<QDockWidget*> dockWidgets;

public slots:
    void updateWidgetList(const QObjectList&);

private slots:
    void doubleClicked(const QModelIndex&);
    void updateTime();
    void focusDockWidget(const QModelIndex&);

signals:
    void createDockWidget(const int&);
};

#endif // MAINWINDOW_H
