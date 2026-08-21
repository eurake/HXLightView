#include "InputTab.h"
#include "Database.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

InputTab::InputTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("数据录入页面", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");

    QPushButton *button = new QPushButton("点击并保存到数据库", this);
    button->setStyleSheet("padding: 10px; font-size: 14px;");

    QLabel *resultLabel = new QLabel("点击次数: 0", this);
    resultLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    layout->addWidget(button);
    layout->addWidget(resultLabel);
    layout->addStretch();

    connect(button, &QPushButton::clicked, this, [resultLabel]() {
        static int count = 0;
        count++;
        resultLabel->setText(QString("点击次数: %1").arg(count));

        if (Database::instance().saveClickRecord(count)) {
            qDebug() << "记录已保存";
        }
    });
}
