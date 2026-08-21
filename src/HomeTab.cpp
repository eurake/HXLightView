#include "HomeTab.h"

#include <QVBoxLayout>
#include <QLabel>

HomeTab::HomeTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("欢迎使用管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");

    QLabel *infoLabel = new QLabel("这是一个左侧菜单+右侧TabView的示例程序", this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 14px; color: #666;");

    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addStretch();
}
