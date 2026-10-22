#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QList>
#include <QHostAddress>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer() override;

    bool start(const QHostAddress &address, quint16 port);
    void stop();
    bool isListening() const;
    QString addressText() const;
    quint16 port() const;
    int clientCount() const;

signals:
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void lineReceived(QTcpSocket *socket, const QString &line);
    void logMessage(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QString peerText(QTcpSocket *socket) const;

    QTcpServer *m_server;
    QList<QTcpSocket *> m_clients;
    QHash<QTcpSocket *, QByteArray> m_buffers;
};

#endif // TCPSERVER_H
