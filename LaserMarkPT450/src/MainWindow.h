#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QWidget>
#include "TcpServer.h"

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QTextEdit;
class QTcpSocket;
class QComboBox;
class QGroupBox;
class QSerialPort;
class QTimer;

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
    void refreshSerialPorts();
    void refreshSerialUi();
    void appendLog(const QString &message);
    void onCreateServer();
    void onCloseServer();
    void onOpenSerial();
    void onCloseSerial();
    void onSerialReadyRead();
    void commitSerialBuffer();
    void applySerialPayload(const QString &text);
    void setAnswerReady(bool ready);
    void onLineReceived(QTcpSocket *socket, const QString &line);
    QString defaultSerialData() const;
    QString currentBody() const;

    TcpServer m_server;
    QSerialPort *m_serial = nullptr;
    QTimer *m_serialIdle = nullptr;
    QByteArray m_serialBuffer;
    bool m_loading = false;

    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpin;
    QPushButton *m_createButton;
    QPushButton *m_closeButton;
    QLabel *m_statusLabel;
    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QPushButton *m_serialRefreshButton;
    QPushButton *m_serialOpenButton;
    QPushButton *m_serialCloseButton;
    QWidget *m_body = nullptr;
    QGroupBox *m_codeGroup = nullptr;
    QLineEdit *m_batchEdit;
    QLineEdit *m_serialDataEdit;
    QLineEdit *m_dateEdit;
    QLabel *m_previewLabel;
    QTextEdit *m_logEdit;
};

#endif // MAINWINDOW_H
