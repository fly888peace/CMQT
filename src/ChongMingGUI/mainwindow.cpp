#include "mainwindow.h"
#include "AppStyle/AppStyle.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pErrorInfoLabel(new QLabel(""))
{
    ui->setupUi(this);
    ui->statusbar->addWidget(m_pErrorInfoLabel);
    this->resize(1000, 600);

    // 阶段 E 在此创建 ControlWidget / ParamWidget / ViewWidget 三个子界面，
    // 塞进 .ui 三个占位 QWidget 的 layout，并连接三路 SigUpdateErrorInfo 信号。

    // 设置标题和图标
    setWindowTitle(QStringLiteral("重明项目-工业相机二次开发-www.roundvision.cc"));
    setWindowIcon(QIcon(":/favicon.ico"));
    // 设置调色板与全局 QSS 样式
    AppStyle::Polish();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnUpdateErrorInfo(QString strErrorInfo)
{
    m_pErrorInfoLabel->setText(strErrorInfo);
    if (!strErrorInfo.isEmpty()) {
        // 弹窗提示
        QMessageBox::critical(this, "Error", strErrorInfo, "Close");
    }
}
