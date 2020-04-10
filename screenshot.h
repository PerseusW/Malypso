#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QScreen>
#include <QPushButton>
#include <QGuiApplication>

class ScreenShotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenShotWidget(QWidget *parent = nullptr);

private:
    QVBoxLayout* getMainLayout();
    QScreen *screen = QGuiApplication::primaryScreen();
    QLabel *label = new QLabel(this);

private slots:
    void captureScreen();
};

#endif // SCREENSHOT_H
