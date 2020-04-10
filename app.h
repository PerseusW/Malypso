#ifndef EXTERNALAPP_H
#define EXTERNALAPP_H

#include <QMap>
#include <QFile>
#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

const static QString PATH = "links";


/*****/


class ExternalAppWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExternalAppWidget(QWidget *parent = nullptr);
    ~ExternalAppWidget();

private:
    QVBoxLayout* getMainLayout();
    void loadData();

    QFile *file = new QFile(PATH,this);
    QListWidget *listWidget = new QListWidget(this);
    QMap<QString, QString> appLinks;

public slots:
    void openApp(const QModelIndex&);
    void addApp();
};

#endif // EXTERNALAPP_H
