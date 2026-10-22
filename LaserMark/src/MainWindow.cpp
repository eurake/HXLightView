#include "MainWindow.h"
#include "AppConfig.h"
#include "Database.h"

#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QSpinBox>
#include <QTcpSocket>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_hostEdit(new QLineEdit(this)),
      m_portSpin(new QSpinBox(this)),
      m_createButton(new QPushButton("创建", this)),
      m_closeButton(new QPushButton("关闭", this)),
      m_statusLabel(new QLabel(this)), m_prefixEdit(new QLineEdit(this)),
      m_dateEdit(new QLineEdit(this)), m_numberSpin(new QSpinBox(this)),
      m_widthSpin(new QSpinBox(this)), m_previewLabel(new QLabel(this)),
      m_logEdit(new QTextEdit(this)) {
  setWindowTitle("LaserMark");
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setFixedWidth(680);
  resize(680, 720);
  QFont font("Microsoft YaHei UI", 10);
  setFont(font);
  buildUi();
  loadSettings();
  refreshStatus();
  refreshPreview();

  connect(m_createButton, &QPushButton::clicked, this,
          [this]() { onCreateServer(); });
  connect(m_closeButton, &QPushButton::clicked, this,
          [this]() { onCloseServer(); });
  connect(m_hostEdit, &QLineEdit::editingFinished, this,
          [this]() { saveSettings(); });
  connect(m_portSpin, qOverload<int>(&QSpinBox::valueChanged), this,
          [this](int) { saveSettings(); });
  connect(m_prefixEdit, &QLineEdit::textChanged, this, [this](const QString &) {
    saveSettings();
    refreshPreview();
  });
  connect(m_numberSpin, qOverload<int>(&QSpinBox::valueChanged), this,
          [this](int) {
            saveSettings();
            refreshPreview();
          });
  connect(m_widthSpin, qOverload<int>(&QSpinBox::valueChanged), this,
          [this](int) {
            saveSettings();
            refreshPreview();
          });
  connect(&m_server, &TcpServer::logMessage, this, &MainWindow::appendLog);
  connect(&m_server, &TcpServer::clientConnected, this,
          [this](const QString &) { refreshStatus(); });
  connect(&m_server, &TcpServer::clientDisconnected, this,
          [this](const QString &) { refreshStatus(); });
  connect(&m_server, &TcpServer::lineReceived, this,
          &MainWindow::onLineReceived);
}

void MainWindow::buildUi() {
  setStyleSheet(
      "QWidget#root { background: #f3f5f8; }"
      "QWidget#header { background: #1e293b; }"
      "QLabel#title { color: white; font-size: 20px; font-weight: 700; }"
      "QLabel#subtitle { color: #94a3b8; font-size: 13px; }"
      "QLabel#section { color: #334155; font-size: 14px; font-weight: 700; }"
      "QLabel#hint { color: #64748b; font-weight: 400; }"
      "QGroupBox {"
      "  background: white;"
      "  border: 1px solid #e2e8f0;"
      "  border-radius: 10px;"
      "  margin-top: 14px;"
      "  padding: 16px 14px 12px 14px;"
      "  font-weight: 600;"
      "  color: #334155;"
      "}"
      "QGroupBox::title {"
      "  subcontrol-origin: margin;"
      "  left: 14px;"
      "  padding: 0 6px;"
      "}"
      "QLineEdit, QSpinBox {"
      "  background: white;"
      "  border: 1px solid #cbd5e1;"
      "  border-radius: 6px;"
      "  padding: 6px 8px;"
      "  min-height: 22px;"
      "  color: #0f172a;"
      "  font-weight: 400;"
      "}"
      "QLineEdit:focus, QSpinBox:focus { border: 1px solid #2563eb; }"
      "QLineEdit:disabled, QSpinBox:disabled { background: #f1f5f9; color: "
      "#64748b; }"
      "QPushButton {"
      "  background: white;"
      "  color: #334155;"
      "  border: 1px solid #cbd5e1;"
      "  border-radius: 6px;"
      "  padding: 8px 16px;"
      "  font-weight: 600;"
      "}"
      "QPushButton:hover { background: #f8fafc; }"
      "QPushButton:disabled { color: #94a3b8; background: #f8fafc; }"
      "QPushButton#createButton { background: #2563eb; color: white; border: "
      "none; }"
      "QPushButton#createButton:hover { background: #1d4ed8; }"
      "QPushButton#createButton:disabled { background: #93c5fd; color: white; }"
      "QPushButton#closeButton { background: #fff7ed; color: #c2410c; border: "
      "1px solid #fdba74; }"
      "QPushButton#closeButton:hover { background: #ffedd5; }"
      "QPushButton#closeButton:disabled { background: #f8fafc; color: #94a3b8; "
      "border: 1px solid #e2e8f0; }"
      "QPushButton#dangerButton { background: white; color: #b91c1c; border: "
      "1px solid #fecaca; }"
      "QPushButton#dangerButton:hover { background: #fef2f2; }"
      "QLabel#preview {"
      "  background: #eff6ff;"
      "  color: #1e3a8a;"
      "  border: 1px solid #bfdbfe;"
      "  border-radius: 8px;"
      "  padding: 10px 12px;"
      "  font-family: Consolas, 'Cascadia Mono', monospace;"
      "  font-size: 16px;"
      "  font-weight: 600;"
      "}"
      "QTextEdit {"
      "  background: #0f172a;"
      "  color: #e2e8f0;"
      "  border: none;"
      "  border-radius: 8px;"
      "  padding: 8px;"
      "  font-family: Consolas, 'Cascadia Mono', monospace;"
      "}");

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  QWidget *header = new QWidget(this);
  header->setObjectName("header");
  QHBoxLayout *headerLayout = new QHBoxLayout(header);
  headerLayout->setContentsMargins(20, 16, 20, 16);
  QLabel *titleLabel = new QLabel("LaserMark", header);
  titleLabel->setObjectName("title");
  QLabel *subtitle = new QLabel("打标应答", header);
  subtitle->setObjectName("subtitle");
  m_statusLabel->setParent(header);
  headerLayout->addWidget(titleLabel);
  headerLayout->addSpacing(10);
  headerLayout->addWidget(subtitle);
  headerLayout->addStretch();
  headerLayout->addWidget(m_statusLabel);

  QWidget *body = new QWidget(this);
  body->setObjectName("root");
  QVBoxLayout *bodyLayout = new QVBoxLayout(body);
  bodyLayout->setContentsMargins(16, 8, 16, 16);
  bodyLayout->setSpacing(12);

  QHBoxLayout *topLayout = new QHBoxLayout();
  topLayout->setSpacing(12);

  QGroupBox *serverGroup = new QGroupBox("TCP 服务器", body);
  QFormLayout *serverLayout = new QFormLayout(serverGroup);
  serverLayout->setLabelAlignment(Qt::AlignRight);
  serverLayout->setHorizontalSpacing(12);
  serverLayout->setVerticalSpacing(10);
  m_hostEdit->setPlaceholderText("127.0.0.1");
  m_hostEdit->setText("127.0.0.1");
  m_hostEdit->setMinimumWidth(120);
  m_portSpin->setRange(1, 65535);
  m_portSpin->setValue(9999);
  m_portSpin->setMinimumWidth(88);
  m_createButton->setObjectName("createButton");
  m_closeButton->setObjectName("closeButton");
  m_createButton->setCursor(Qt::PointingHandCursor);
  m_closeButton->setCursor(Qt::PointingHandCursor);
  QHBoxLayout *buttonRow = new QHBoxLayout();
  buttonRow->addWidget(m_createButton);
  buttonRow->addWidget(m_closeButton);
  buttonRow->addStretch();
  serverLayout->addRow("IP", m_hostEdit);
  serverLayout->addRow("端口", m_portSpin);
  serverLayout->addRow("", buttonRow);

  QGroupBox *codeGroup = new QGroupBox("应答内容", body);
  QFormLayout *codeLayout = new QFormLayout(codeGroup);
  codeLayout->setLabelAlignment(Qt::AlignRight);
  codeLayout->setHorizontalSpacing(12);
  codeLayout->setVerticalSpacing(10);
  // m_prefixEdit->setPlaceholderText("例如 33310003S");
  m_prefixEdit->setText("33310003S");
  m_dateEdit->setReadOnly(true);
  m_dateEdit->setText(currentDateCode());
  m_numberSpin->setRange(0, 999999999);
  m_numberSpin->setValue(1);
  m_widthSpin->setRange(1, 12);
  m_widthSpin->setValue(5);
  m_previewLabel->setObjectName("preview");
  m_previewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_previewLabel->setMinimumHeight(42);
  QLabel *hint = new QLabel(
      "收到 QRCODE 即返回编码，不附加 \\r\\n，C 段随后加 1", codeGroup);
  hint->setObjectName("hint");
  codeLayout->addRow("A 段", m_prefixEdit);
  codeLayout->addRow("B 段日期", m_dateEdit);
  codeLayout->addRow("C 段", m_numberSpin);
  codeLayout->addRow("C 段长度", m_widthSpin);
  codeLayout->addRow("预览", m_previewLabel);
  codeLayout->addRow("", hint);

  topLayout->addWidget(serverGroup, 1);
  topLayout->addWidget(codeGroup, 1);

  QHBoxLayout *logHeader = new QHBoxLayout();
  QLabel *logTitle = new QLabel("运行日志", body);
  logTitle->setObjectName("section");
  QPushButton *clearLogButton = new QPushButton("清空日志", body);
  clearLogButton->setCursor(Qt::PointingHandCursor);
  logHeader->addWidget(logTitle);
  logHeader->addStretch();
  logHeader->addWidget(clearLogButton);

  m_logEdit->setReadOnly(true);
  m_logEdit->setPlaceholderText("连接和收发日志...");
  QPalette logPalette = m_logEdit->palette();
  logPalette.setColor(QPalette::PlaceholderText, QColor("#94a3b8"));
  m_logEdit->setPalette(logPalette);

  bodyLayout->addLayout(topLayout);
  bodyLayout->addLayout(logHeader);
  bodyLayout->addWidget(m_logEdit, 1);

  layout->addWidget(header);
  layout->addWidget(body, 1);

  connect(clearLogButton, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
}

void MainWindow::loadSettings() {
  const MarkSettings settings = AppConfig::load();
  m_hostEdit->setText(settings.host);
  m_portSpin->setValue(settings.port);
  m_prefixEdit->setText(settings.prefix);
  m_numberSpin->setValue(settings.number);
  m_widthSpin->setValue(settings.width);
}

void MainWindow::saveSettings() {
  MarkSettings settings;
  settings.host = m_hostEdit->text().trimmed();
  settings.port = m_portSpin->value();
  settings.prefix = m_prefixEdit->text();
  settings.number = m_numberSpin->value();
  settings.width = m_widthSpin->value();
  AppConfig::save(settings);
}

void MainWindow::refreshStatus() {
  const QString badge =
      "border-radius: 11px; padding: 4px 12px; font-weight: 600;";
  if (m_server.isListening()) {
    m_statusLabel->setText(QString("监听中  %1:%2  ·  %3 个连接")
                               .arg(m_server.addressText())
                               .arg(m_server.port())
                               .arg(m_server.clientCount()));
    m_statusLabel->setStyleSheet(badge +
                                 "background: #dcfce7; color: #166534;");
    m_createButton->setEnabled(false);
    m_closeButton->setEnabled(true);
    m_hostEdit->setEnabled(false);
    m_portSpin->setEnabled(false);
  } else {
    m_statusLabel->setText("未启动");
    m_statusLabel->setStyleSheet(badge +
                                 "background: #e2e8f0; color: #475569;");
    m_createButton->setEnabled(true);
    m_closeButton->setEnabled(false);
    m_hostEdit->setEnabled(true);
    m_portSpin->setEnabled(true);
  }
}

void MainWindow::refreshPreview() {
  m_dateEdit->setText(currentDateCode());
  m_previewLabel->setText(currentBody());
}

void MainWindow::appendLog(const QString &message) {
  const QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
  m_logEdit->append(QString("[%1] %2").arg(time, message));
}

void MainWindow::onCreateServer() {
  const QString host = m_hostEdit->text().trimmed();
  QHostAddress address;
  if (!address.setAddress(host)) {
    appendLog("IP 地址无效: " + host);
    return;
  }

  saveSettings();
  if (m_server.start(address, static_cast<quint16>(m_portSpin->value()))) {
    refreshStatus();
  }
}

void MainWindow::onCloseServer() {
  m_server.stop();
  refreshStatus();
}

void MainWindow::onLineReceived(QTcpSocket *socket, const QString &line) {
  if (!socket) {
    return;
  }

  QString command;
  for (const QChar &ch : line) {
    if (ch.unicode() > 32) {
      command.append(ch);
    }
  }
  if (command != "QRCODE") {
    appendLog(QString("未返回，解析到: %1").arg(command));
    return;
  }

  const QString peer = QString("%1:%2")
                           .arg(socket->peerAddress().toString())
                           .arg(socket->peerPort());
  const QString body = currentBody();
  const qint64 written = socket->write(body.toUtf8());
  socket->flush();
  appendLog(
      QString("已返回 [%1] %2 字节: %3").arg(peer).arg(written).arg(body));

  if (!Database::instance().saveMark(body)) {
    appendLog("写入数据库失败");
  }

  m_numberSpin->setValue(m_numberSpin->value() + 1);
}

QString MainWindow::currentDateCode() const {
  return QDate::currentDate().toString("yyMMdd");
}

QString MainWindow::currentBody() const {
  const QString number = QString("%1").arg(
      m_numberSpin->value(), m_widthSpin->value(), 10, QChar('0'));
  return m_prefixEdit->text() + currentDateCode() + number;
}
