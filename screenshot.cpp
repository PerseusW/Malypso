#include "screenshot.h"

ScreenShotWidget::ScreenShotWidget(QWidget *parent) : QWidget(parent)
{
    this->setLayout(getMainLayout());
}

QVBoxLayout *ScreenShotWidget::getMainLayout()
{
    QVBoxLayout *layout = new QVBoxLayout();
    QPushButton *button = new QPushButton("Capture",this);
    layout->addWidget(label);
    layout->addWidget(button);
    connect(button,&QPushButton::clicked,this,&ScreenShotWidget::captureScreen);
    return layout;
}

void ScreenShotWidget::captureScreen()
{
    QPixmap pixmap = screen->grabWindow(0);
    label->setPixmap(pixmap.scaledToWidth(400));

}
