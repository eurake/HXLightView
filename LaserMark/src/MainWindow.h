#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "TcpServer.h"

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QTextEdit;
class QTcpSocket;

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void loadSettings();
    void saveSettings();
    void refreshStatus();
    void refreshPreview();
    void appendLog(const QString &message);
    void onCreateServer();
    void onCloseServer();
    void onLineReceived(QTcpSocket *socket, const QString &line);
    QString currentDateCode() const;
    QString currentBody() const;

    TcpServer m_server;
    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpin;
    QPushButton *m_createButton;
    QPushButton *m_closeButton;
    QLabel *m_statusLabel;
    QLineEdit *m_prefixEdit;
    QLineEdit *m_dateEdit;
    QSpinBox *m_numberSpin;
    QSpinBox *m_widthSpin;
    QLabel *m_previewLabel;
    QTextEdit *m_logEdit;
};

#endif // MAINWINDOW_H
