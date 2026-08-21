#include "QueryTab.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSqlQuery>

QueryTab::QueryTab(QWidget *parent)
    : QWidget(parent)
    , m_tableWidget(new QTableWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("数据查询页面", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");

    m_tableWidget->setColumnCount(3);
    m_tableWidget->setHorizontalHeaderLabels({"ID", "点击次数", "时间"});
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setAlternatingRowColors(true);

    QPushButton *refreshButton = new QPushButton("刷新数据", this);

    layout->addWidget(label);
    layout->addWidget(refreshButton);
    layout->addWidget(m_tableWidget);

    connect(refreshButton, &QPushButton::clicked, this, &QueryTab::refreshTable);
    refreshTable();
}

void QueryTab::refreshTable()
{
    QSqlQuery query;
    query.exec("SELECT id, click_count, click_time FROM click_records ORDER BY id DESC LIMIT 100");

    m_tableWidget->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
}
