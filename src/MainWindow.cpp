#include "MainWindow.h"
#include "HomeTab.h"
#include "InputTab.h"
#include "QueryTab.h"
#include "TcpServerTab.h"
#include "SettingsTab.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTabWidget>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("管理系统 - 左侧菜单 + 右侧TabView");
    resize(900, 600);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *leftMenu = new QWidget(this);
    leftMenu->setFixedWidth(200);
    leftMenu->setStyleSheet("background-color: #2c3e50;");

    QVBoxLayout *menuLayout = new QVBoxLayout(leftMenu);
    menuLayout->setSpacing(0);
    menuLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *menuTitle = new QLabel("功能菜单", leftMenu);
    menuTitle->setAlignment(Qt::AlignCenter);
    menuTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold; padding: 20px;");

    QListWidget *menuList = new QListWidget(leftMenu);
    menuList->setStyleSheet(
        "QListWidget {"
        "   background-color: #34495e;"
        "   color: white;"
        "   border: none;"
        "   font-size: 14px;"
        "}"
        "QListWidget::item {"
        "   padding: 15px;"
        "   border-bottom: 1px solid #2c3e50;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #3498db;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #2980b9;"
        "   border-left: 3px solid #e74c3c;"
        "}"
    );

    menuList->addItem("🏠 首页");
    menuList->addItem("📝 数据录入");
    menuList->addItem("📊 数据查询");
    menuList->addItem("🔌 TCP 服务器");
    menuList->addItem("⚙️ 设置");

    menuLayout->addWidget(menuTitle);
    menuLayout->addWidget(menuList);

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: 1px solid #ccc;"
        "   background: white;"
        "}"
        "QTabBar::tab {"
        "   padding: 10px 20px;"
        "   font-size: 14px;"
        "}"
        "QTabBar::tab:selected {"
        "   background-color: #3498db;"
        "   color: white;"
        "}"
    );

    tabWidget->addTab(new HomeTab(this), "首页");
    tabWidget->addTab(new InputTab(this), "数据录入");
    tabWidget->addTab(new QueryTab(this), "数据查询");
    tabWidget->addTab(new TcpServerTab(&m_tcpServer, this), "TCP 服务器");
    tabWidget->addTab(new SettingsTab(this), "设置");

    connect(menuList, &QListWidget::currentRowChanged, this, [tabWidget](int row) {
        if (row >= 0 && row < tabWidget->count()) {
            tabWidget->setCurrentIndex(row);
        }
    });
    connect(tabWidget, &QTabWidget::currentChanged, this, [menuList](int index) {
        menuList->setCurrentRow(index);
    });

    menuList->setCurrentRow(0);

    mainLayout->addWidget(leftMenu);
    mainLayout->addWidget(tabWidget);
}
