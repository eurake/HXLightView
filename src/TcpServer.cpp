#include "TcpServer.h"

#include <QHostAddress>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::start(quint16 port)
{
    if (m_server->isListening()) {
        emit logMessage(QString("服务器已在端口 %1 上运行").arg(m_server->serverPort()));
        return true;
    }

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("启动失败: %1").arg(m_server->errorString()));
        return false;
    }

    emit logMessage(QString("TCP 服务器已启动，监听端口 %1").arg(m_server->serverPort()));
    return true;
}

void TcpServer::stop()
{
    for (QTcpSocket *socket : m_clients) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_clients.clear();

    if (m_server->isListening()) {
        m_server->close();
        emit logMessage("TCP 服务器已停止");
    }
}

bool TcpServer::isListening() const
{
    return m_server->isListening();
}

quint16 TcpServer::port() const
{
    return m_server->serverPort();
}

int TcpServer::clientCount() const
{
    return m_clients.size();
}

void TcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        m_clients.append(socket);

        connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onDisconnected);

        const QString address = peerText(socket);
        emit clientConnected(address);
        emit logMessage(QString("客户端已连接: %1  当前连接数: %2")
                            .arg(address)
                            .arg(m_clients.size()));
    }
}

void TcpServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    const QByteArray data = socket->readAll();
    const QString address = peerText(socket);
    emit dataReceived(address, data);
    emit logMessage(QString("收到 [%1]: %2")
                        .arg(address, QString::fromUtf8(data)));

    socket->write(data);
}

void TcpServer::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    const QString address = peerText(socket);
    m_clients.removeAll(socket);
    socket->deleteLater();

    emit clientDisconnected(address);
    emit logMessage(QString("客户端已断开: %1  当前连接数: %2")
                        .arg(address)
                        .arg(m_clients.size()));
}

QString TcpServer::peerText(QTcpSocket *socket) const
{
    return QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
}
