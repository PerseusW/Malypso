#include "app.h"

ExternalAppWidget::ExternalAppWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setLayout(getMainLayout());
    this->loadData();
}

ExternalAppWidget::~ExternalAppWidget()
{
    file->open(QFile::WriteOnly|QFile::Truncate);
    QTextStream outputStream(file);
    QMapIterator<QString,QString> it(appLinks);
    while (it.hasNext()) {
        it.next();
        outputStream << it.key() << "\t" << it.value() << endl;
    }
    file->close();
}

QVBoxLayout *ExternalAppWidget::getMainLayout()
{
    QVBoxLayout *layout = new QVBoxLayout();
    QPushButton *button = new QPushButton("Add Application",this);
    layout->addWidget(listWidget);
    layout->addWidget(button);
    connect(listWidget,&QListWidget::doubleClicked,this,&ExternalAppWidget::openApp);
    connect(button,&QPushButton::clicked,this,&ExternalAppWidget::addApp);
    return layout;
}

void ExternalAppWidget::loadData()
{
    if (file->open(QFile::ReadOnly)) {
        QTextStream inputStream(file);
        while (!inputStream.atEnd()) {
            QString link = inputStream.readLine();
            QStringList tuple = link.split("\t");
            appLinks.insert(tuple.first(),tuple.last());
        }
        file->close();
    }
    for (QString& appName: appLinks.keys()) {
        listWidget->addItem(appName);
    }
}

void ExternalAppWidget::openApp(const QModelIndex &index)
{
    system(appLinks.value(index.data().toString()).toLatin1() + '&');
}

void ExternalAppWidget::addApp()
{
    QString appPath = QFileDialog::getOpenFileName(this,"Select Application","C:/Users/user/Desktop","*.exe");
    if (appPath.isEmpty()) {return;}
    QString appName = appPath.split("/").last().split(".").first();
    if (!appLinks.values().contains(appPath)) {
        appLinks.insert(appName,appPath);
        listWidget->clear();
        for (QString appName: appLinks.keys()) {
            listWidget->addItem(appName);
        }
    }
    else {
        QMessageBox::information(this,"Conflict","Application path already exists.");
    }
}
