#ifndef TCPSERVERTAB_H
#define TCPSERVERTAB_H

#include <QWidget>

class TcpServer;
class QSpinBox;
class QPushButton;
class QLabel;
class QTextEdit;

class TcpServerTab : public QWidget
{
public:
    explicit TcpServerTab(TcpServer *server, QWidget *parent = nullptr);

private:
    void appendLog(const QString &message);
    void refreshStatus();

    TcpServer *m_server;
    QSpinBox *m_portSpin;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QLabel *m_statusLabel;
    QTextEdit *m_logEdit;
};

#endif // TCPSERVERTAB_H
