#include "MainWindow.h"
#include "AppConfig.h"
#include "Database.h"
#include "Version.h"

#include <QComboBox>
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
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QTcpSocket>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_hostEdit(new QLineEdit(this)),
      m_portSpin(new QSpinBox(this)),
      m_createButton(new QPushButton("创建", this)),
      m_closeButton(new QPushButton("关闭", this)),
      m_statusLabel(new QLabel(this)), m_portCombo(new QComboBox(this)),
      m_baudCombo(new QComboBox(this)),
      m_serialRefreshButton(new QPushButton("刷新", this)),
      m_serialOpenButton(new QPushButton("打开", this)),
      m_serialCloseButton(new QPushButton("关闭", this)),
      m_batchEdit(new QLineEdit(this)), m_serialDataEdit(new QLineEdit(this)),
      m_dateEdit(new QLineEdit(this)), m_previewLabel(new QLabel(this)),
      m_logEdit(new QTextEdit(this)), m_serial(new QSerialPort(this)),
      m_serialIdle(new QTimer(this)) {
  setWindowTitle(QString("LaserMarkPT450  V%1").arg(APP_VERSION));
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setFixedWidth(680);
  resize(680, 760);
  QFont font("Microsoft YaHei UI", 10);
  setFont(font);
  m_serialIdle->setSingleShot(true);
  m_serialIdle->setInterval(80);
  buildUi();
  loadSettings();
  refreshSerialPorts();
  refreshStatus();
  refreshSerialUi();
  refreshPreview();

  connect(m_createButton, &QPushButton::clicked, this,
          [this]() { onCreateServer(); });
  connect(m_closeButton, &QPushButton::clicked, this,
          [this]() { onCloseServer(); });
  connect(m_hostEdit, &QLineEdit::editingFinished, this,
          [this]() { saveSettings(); });
  connect(m_portSpin, qOverload<int>(&QSpinBox::valueChanged), this,
          [this](int) { saveSettings(); });
  connect(m_serialRefreshButton, &QPushButton::clicked, this,
          [this]() { refreshSerialPorts(); });
  connect(m_serialOpenButton, &QPushButton::clicked, this,
          [this]() { onOpenSerial(); });
  connect(m_serialCloseButton, &QPushButton::clicked, this,
          [this]() { onCloseSerial(); });
  connect(m_portCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &) { saveSettings(); });
  connect(m_baudCombo, &QComboBox::currentTextChanged, this,
          [this](const QString &) { saveSettings(); });
  connect(m_batchEdit, &QLineEdit::textChanged, this, [this](const QString &) {
    saveSettings();
    refreshPreview();
  });
  connect(m_dateEdit, &QLineEdit::textChanged, this, [this](const QString &) {
    saveSettings();
    refreshPreview();
  });
  connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::onSerialReadyRead);
  connect(m_serialIdle, &QTimer::timeout, this, &MainWindow::commitSerialBuffer);
  connect(m_serial, &QSerialPort::errorOccurred, this,
          [this](QSerialPort::SerialPortError error) {
            if (error == QSerialPort::NoError ||
                error == QSerialPort::TimeoutError) {
              return;
            }
            appendLog("串口错误: " + m_serial->errorString());
            if (!m_serial->isOpen()) {
              refreshSerialUi();
            }
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
      "QWidget#root { background: #e5e7eb; }"
      "QWidget#root[fresh=\"true\"] { background: #bbf7d0; }"
      "QWidget#header { background: #134e4a; }"
      "QLabel#title { color: white; font-size: 20px; font-weight: 700; }"
      "QLabel#subtitle { color: #99f6e4; font-size: 13px; }"
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
      "QGroupBox#codeGroup {"
      "  background: #d1d5db;"
      "  border: 2px solid #9ca3af;"
      "}"
      "QGroupBox#codeGroup[fresh=\"true\"] {"
      "  background: #86efac;"
      "  border: 2px solid #15803d;"
      "}"
      "QLineEdit, QSpinBox, QComboBox {"
      "  background: white;"
      "  border: 1px solid #cbd5e1;"
      "  border-radius: 6px;"
      "  padding: 6px 8px;"
      "  min-height: 22px;"
      "  color: #0f172a;"
      "  font-weight: 400;"
      "}"
      "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid "
      "#0f766e; }"
      "QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled { background: "
      "#f1f5f9; color: #64748b; }"
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
      "QPushButton#createButton, QPushButton#serialOpenButton { background: "
      "#0f766e; color: white; border: none; }"
      "QPushButton#createButton:hover, QPushButton#serialOpenButton:hover { "
      "background: #115e59; }"
      "QPushButton#createButton:disabled, QPushButton#serialOpenButton:disabled "
      "{ background: #5eead4; color: white; }"
      "QPushButton#closeButton, QPushButton#serialCloseButton { background: "
      "#fff7ed; color: #c2410c; border: 1px solid #fdba74; }"
      "QPushButton#closeButton:hover, QPushButton#serialCloseButton:hover { "
      "background: #ffedd5; }"
      "QPushButton#closeButton:disabled, "
      "QPushButton#serialCloseButton:disabled { background: #f8fafc; color: "
      "#94a3b8; border: 1px solid #e2e8f0; }"
      "QLabel#preview {"
      "  background: #f0fdfa;"
      "  color: #134e4a;"
      "  border: 1px solid #99f6e4;"
      "  border-radius: 8px;"
      "  padding: 10px 12px;"
      "  font-family: Consolas, 'Cascadia Mono', monospace;"
      "  font-size: 15px;"
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
  QLabel *titleLabel = new QLabel("LaserMarkPT450", header);
  titleLabel->setObjectName("title");
  QLabel *subtitle = new QLabel(QString("V%1").arg(APP_VERSION), header);
  subtitle->setObjectName("subtitle");
  m_statusLabel->setParent(header);
  headerLayout->addWidget(titleLabel);
  headerLayout->addSpacing(10);
  headerLayout->addWidget(subtitle);
  headerLayout->addStretch();
  headerLayout->addWidget(m_statusLabel);

  QWidget *body = new QWidget(this);
  body->setObjectName("root");
  m_body = body;
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
  codeGroup->setObjectName("codeGroup");
  m_codeGroup = codeGroup;
  QFormLayout *codeLayout = new QFormLayout(codeGroup);
  codeLayout->setLabelAlignment(Qt::AlignRight);
  codeLayout->setHorizontalSpacing(12);
  codeLayout->setVerticalSpacing(10);
  m_batchEdit->setPlaceholderText("例如 DE525017");
  m_serialDataEdit->setReadOnly(true);
  m_serialDataEdit->setText(defaultSerialData());
  m_dateEdit->setPlaceholderText("yyMMdd");
  m_dateEdit->setMaxLength(12);
  m_previewLabel->setObjectName("preview");
  m_previewLabel->setWordWrap(true);
  m_previewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_previewLabel->setMinimumHeight(42);
  QLabel *hint = new QLabel(
      "未读到串口时中间段为 010101010101。读到数据后应答区变绿，TCP 返回后变灰",
      codeGroup);
  hint->setObjectName("hint");
  hint->setWordWrap(true);
  codeLayout->addRow("订单批号", m_batchEdit);
  codeLayout->addRow("串口数据", m_serialDataEdit);
  codeLayout->addRow("日期", m_dateEdit);
  codeLayout->addRow("预览", m_previewLabel);
  codeLayout->addRow("", hint);

  topLayout->addWidget(serverGroup, 1);
  topLayout->addWidget(codeGroup, 1);

  QGroupBox *serialGroup = new QGroupBox("串口", body);
  QHBoxLayout *serialLayout = new QHBoxLayout(serialGroup);
  serialLayout->setSpacing(8);
  m_portCombo->setMinimumWidth(110);
  const int bauds[] = {9600, 19200, 38400, 57600, 115200};
  for (int baud : bauds) {
    m_baudCombo->addItem(QString::number(baud));
  }
  m_baudCombo->setCurrentText("9600");
  m_baudCombo->setMinimumWidth(96);
  m_serialRefreshButton->setCursor(Qt::PointingHandCursor);
  m_serialOpenButton->setObjectName("serialOpenButton");
  m_serialCloseButton->setObjectName("serialCloseButton");
  m_serialOpenButton->setCursor(Qt::PointingHandCursor);
  m_serialCloseButton->setCursor(Qt::PointingHandCursor);
  serialLayout->addWidget(new QLabel("端口", serialGroup));
  serialLayout->addWidget(m_portCombo);
  serialLayout->addWidget(new QLabel("波特率", serialGroup));
  serialLayout->addWidget(m_baudCombo);
  serialLayout->addWidget(m_serialRefreshButton);
  serialLayout->addWidget(m_serialOpenButton);
  serialLayout->addWidget(m_serialCloseButton);
  serialLayout->addStretch();

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
  bodyLayout->addWidget(serialGroup);
  bodyLayout->addLayout(logHeader);
  bodyLayout->addWidget(m_logEdit, 1);

  layout->addWidget(header);
  layout->addWidget(body, 1);

  connect(clearLogButton, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
  setAnswerReady(false);
}

void MainWindow::loadSettings() {
  m_loading = true;
  const MarkSettings settings = AppConfig::load();
  m_hostEdit->setText(settings.host);
  m_portSpin->setValue(settings.port);
  m_batchEdit->setText(settings.batch);
  m_dateEdit->setText(settings.date.isEmpty()
                          ? QDate::currentDate().toString("yyMMdd")
                          : settings.date);
  if (!settings.serialPort.isEmpty()) {
    m_portCombo->addItem(settings.serialPort);
    m_portCombo->setCurrentText(settings.serialPort);
  }
  m_baudCombo->setCurrentText(QString::number(settings.baud));
  m_loading = false;
}

void MainWindow::saveSettings() {
  if (m_loading) {
    return;
  }
  MarkSettings settings;
  settings.host = m_hostEdit->text().trimmed();
  settings.port = m_portSpin->value();
  settings.batch = m_batchEdit->text();
  settings.date = m_dateEdit->text().trimmed();
  settings.serialPort = m_portCombo->currentText().trimmed();
  settings.baud = m_baudCombo->currentText().toInt();
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
  m_previewLabel->setText(currentBody());
}

void MainWindow::refreshSerialPorts() {
  const QString current = m_portCombo->currentText();
  m_loading = true;
  m_portCombo->clear();
  const auto ports = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : ports) {
    m_portCombo->addItem(info.portName());
  }
  if (!current.isEmpty() && m_portCombo->findText(current) < 0) {
    m_portCombo->addItem(current);
  }
  if (!current.isEmpty()) {
    m_portCombo->setCurrentText(current);
  }
  m_loading = false;
}

void MainWindow::refreshSerialUi() {
  const bool open = m_serial->isOpen();
  m_serialOpenButton->setEnabled(!open && m_portCombo->count() > 0);
  m_serialCloseButton->setEnabled(open);
  m_portCombo->setEnabled(!open);
  m_baudCombo->setEnabled(!open);
  m_serialRefreshButton->setEnabled(!open);
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

void MainWindow::onOpenSerial() {
  const QString portName = m_portCombo->currentText().trimmed();
  if (portName.isEmpty()) {
    appendLog("请选择串口");
    return;
  }

  saveSettings();
  m_serialBuffer.clear();
  m_serial->setPortName(portName);
  m_serial->setBaudRate(m_baudCombo->currentText().toInt());
  m_serial->setDataBits(QSerialPort::Data8);
  m_serial->setParity(QSerialPort::NoParity);
  m_serial->setStopBits(QSerialPort::OneStop);
  m_serial->setFlowControl(QSerialPort::NoFlowControl);
  if (!m_serial->open(QIODevice::ReadWrite)) {
    appendLog(QString("打开串口 %1 失败: %2")
                  .arg(portName, m_serial->errorString()));
    refreshSerialUi();
    return;
  }
  appendLog(QString("串口已打开 %1  %2")
                .arg(portName)
                .arg(m_serial->baudRate()));
  refreshSerialUi();
}

void MainWindow::onCloseSerial() {
  m_serialIdle->stop();
  m_serialBuffer.clear();
  if (m_serial->isOpen()) {
    m_serial->close();
  }
  appendLog("串口已关闭");
  refreshSerialUi();
}

void MainWindow::onSerialReadyRead() {
  m_serialBuffer.append(m_serial->readAll());
  bool committed = false;
  while (true) {
    const int n = m_serialBuffer.indexOf('\n');
    const int r = m_serialBuffer.indexOf('\r');
    int idx = -1;
    if (n < 0) {
      idx = r;
    } else if (r < 0) {
      idx = n;
    } else {
      idx = qMin(n, r);
    }
    if (idx < 0) {
      break;
    }
    const QByteArray line = m_serialBuffer.left(idx).trimmed();
    m_serialBuffer.remove(0, idx + 1);
    if (!m_serialBuffer.isEmpty() &&
        (m_serialBuffer.at(0) == '\n' || m_serialBuffer.at(0) == '\r')) {
      m_serialBuffer.remove(0, 1);
    }
    if (!line.isEmpty()) {
      applySerialPayload(QString::fromUtf8(line));
      committed = true;
    }
  }
  if (m_serialBuffer.trimmed().isEmpty()) {
    m_serialBuffer.clear();
    if (committed) {
      m_serialIdle->stop();
    }
    return;
  }
  m_serialIdle->start();
}

void MainWindow::commitSerialBuffer() {
  const QByteArray chunk = m_serialBuffer.trimmed();
  m_serialBuffer.clear();
  if (!chunk.isEmpty()) {
    applySerialPayload(QString::fromUtf8(chunk));
  }
}

void MainWindow::applySerialPayload(const QString &text) {
  m_serialDataEdit->setText(text);
  appendLog("串口数据: " + text);
  setAnswerReady(true);
  refreshPreview();
}

void MainWindow::setAnswerReady(bool ready) {
  m_body->setProperty("fresh", ready);
  m_codeGroup->setProperty("fresh", ready);
  m_codeGroup->setTitle(ready ? "应答内容  已读到串口数据" : "应答内容");
  const auto repolish = [](QWidget *widget) {
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
  };
  repolish(m_body);
  repolish(m_codeGroup);
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

  setAnswerReady(false);

  if (!Database::instance().saveMark(body)) {
    appendLog("写入数据库失败");
  }
}

QString MainWindow::defaultSerialData() const {
  const QString pattern = QStringLiteral("0101");
  const int length = QStringLiteral("0224A0044030").size();
  QString value;
  while (value.size() < length) {
    value += pattern;
  }
  return value.left(length);
}

QString MainWindow::currentBody() const {
  const QString batch = m_batchEdit->text().trimmed();
  QString serial = m_serialDataEdit->text().trimmed();
  if (serial.isEmpty()) {
    serial = defaultSerialData();
  }
  QString date = m_dateEdit->text().trimmed();
  if (date.isEmpty()) {
    date = QDate::currentDate().toString("yyMMdd");
  }
  return batch + serial + date;
}
