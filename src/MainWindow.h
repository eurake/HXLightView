#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "TcpServer.h"

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    TcpServer m_tcpServer;
};

#endif // MAINWINDOW_H
