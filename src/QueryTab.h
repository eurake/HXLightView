#ifndef QUERYTAB_H
#define QUERYTAB_H

#include <QWidget>

class QTableWidget;

class QueryTab : public QWidget
{
public:
    explicit QueryTab(QWidget *parent = nullptr);

private:
    void refreshTable();

    QTableWidget *m_tableWidget;
};

#endif // QUERYTAB_H
