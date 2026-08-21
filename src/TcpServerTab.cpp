#include "TcpServerTab.h"
#include "TcpServer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QDateTime>

TcpServerTab::TcpServerTab(TcpServer *server, QWidget *parent)
    : QWidget(parent)
    , m_server(server)
    , m_portSpin(new QSpinBox(this))
    , m_startButton(new QPushButton("启动服务器", this))
    , m_stopButton(new QPushButton("停止服务器", this))
    , m_statusLabel(new QLabel("状态: 未启动", this))
    , m_logEdit(new QTextEdit(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("TCP 服务器", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");

    QGroupBox *controlGroup = new QGroupBox("服务控制", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    QLabel *portLabel = new QLabel("监听端口:", controlGroup);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(8888);

    m_startButton->setStyleSheet("padding: 8px 16px;");
    m_stopButton->setStyleSheet("padding: 8px 16px;");
    m_stopButton->setEnabled(false);
    m_statusLabel->setStyleSheet("color: #666;");

    controlLayout->addWidget(portLabel);
    controlLayout->addWidget(m_portSpin);
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_statusLabel);
    controlLayout->addStretch();

    QLabel *logTitle = new QLabel("运行日志", this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setPlaceholderText("服务器日志将显示在这里...");

    QPushButton *clearLogButton = new QPushButton("清空日志", this);

    layout->addWidget(titleLabel);
    layout->addWidget(controlGroup);
    layout->addWidget(logTitle);
    layout->addWidget(m_logEdit);
    layout->addWidget(clearLogButton);

    connect(m_startButton, &QPushButton::clicked, this, [this]() {
        m_server->start(static_cast<quint16>(m_portSpin->value()));
        refreshStatus();
    });
    connect(m_stopButton, &QPushButton::clicked, this, [this]() {
        m_server->stop();
        refreshStatus();
    });
    connect(clearLogButton, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
    connect(m_server, &TcpServer::logMessage, this, &TcpServerTab::appendLog);
    connect(m_server, &TcpServer::clientConnected, this, [this](const QString &) {
        refreshStatus();
    });
    connect(m_server, &TcpServer::clientDisconnected, this, [this](const QString &) {
        refreshStatus();
    });
}

void TcpServerTab::appendLog(const QString &message)
{
    const QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(time, message));
}

void TcpServerTab::refreshStatus()
{
    if (m_server->isListening()) {
        m_statusLabel->setText(QString("状态: 监听中  端口 %1  连接数 %2")
                                   .arg(m_server->port())
                                   .arg(m_server->clientCount()));
        m_statusLabel->setStyleSheet("color: green;");
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_portSpin->setEnabled(false);
    } else {
        m_statusLabel->setText("状态: 未启动");
        m_statusLabel->setStyleSheet("color: #666;");
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        m_portSpin->setEnabled(true);
    }
}
