#include "TcpServer.h"

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

bool TcpServer::start(const QHostAddress &address, quint16 port)
{
    if (m_server->isListening()) {
        emit logMessage(QString("服务器已在 %1:%2 上运行")
                            .arg(m_server->serverAddress().toString())
                            .arg(m_server->serverPort()));
        return true;
    }

    if (!m_server->listen(address, port)) {
        emit logMessage(QString("启动失败: %1").arg(m_server->errorString()));
        return false;
    }

    emit logMessage(QString("TCP 服务器已启动，监听 %1:%2")
                        .arg(m_server->serverAddress().toString())
                        .arg(m_server->serverPort()));
    return true;
}

void TcpServer::stop()
{
    for (QTcpSocket *socket : m_clients) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_clients.clear();
    m_buffers.clear();

    if (m_server->isListening()) {
        m_server->close();
        emit logMessage("TCP 服务器已关闭");
    }
}

bool TcpServer::isListening() const
{
    return m_server->isListening();
}

QString TcpServer::addressText() const
{
    return m_server->serverAddress().toString();
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
        m_buffers.insert(socket, QByteArray());

        socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onDisconnected);

        const QString address = peerText(socket);
        emit clientConnected(address);
        emit logMessage(QString("客户端已连接: %1  当前连接数: %2")
                            .arg(address)
                            .arg(m_clients.size()));
    }
}

static bool takeQrCodeRequest(QByteArray *pending)
{
    const QByteArray pattern("QRCODE");
    const int pos = pending->indexOf(pattern);
    if (pos < 0) {
        return false;
    }

    pending->remove(0, pos + pattern.size());
    return true;
}

void TcpServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket || !m_buffers.contains(socket)) {
        return;
    }

    const QByteArray data = socket->readAll();
    QString shown = QString::fromUtf8(data);
    shown.replace(QLatin1String("\r"), QLatin1String("\\r"));
    shown.replace(QLatin1String("\n"), QLatin1String("\\n"));
    QString hex;
    for (int i = 0; i < data.size(); ++i) {
        hex += QString("%1 ").arg(quint8(data.at(i)), 2, 16, QChar('0'));
    }
    emit logMessage(QString("收到 [%1]: %2  [%3]")
                        .arg(peerText(socket), shown, hex.trimmed()));

    QByteArray pending = m_buffers.value(socket);
    pending.append(data);
    while (takeQrCodeRequest(&pending)) {
        emit lineReceived(socket, QStringLiteral("QRCODE"));
    }
    m_buffers[socket] = pending;
}

void TcpServer::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    const QString address = peerText(socket);
    m_clients.removeAll(socket);
    m_buffers.remove(socket);
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
