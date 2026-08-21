#include "SettingsTab.h"
#include "Database.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SettingsTab::SettingsTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("设置页面", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");

    QPushButton *clearButton = new QPushButton("清空所有数据", this);
    clearButton->setStyleSheet("padding: 10px; background-color: #f44336; color: white;");

    QLabel *statusLabel = new QLabel("", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    layout->addWidget(clearButton);
    layout->addWidget(statusLabel);
    layout->addStretch();

    connect(clearButton, &QPushButton::clicked, this, [statusLabel]() {
        QString error;
        if (Database::instance().clearClickRecords(&error)) {
            statusLabel->setText("数据已清空");
            statusLabel->setStyleSheet("color: green;");
        } else {
            statusLabel->setText("清空失败: " + error);
            statusLabel->setStyleSheet("color: red;");
        }
    });
}
