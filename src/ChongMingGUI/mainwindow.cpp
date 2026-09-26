#include "mainwindow.h"
#include "AppStyle/AppStyle.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pControlWidget(new ControlWidget())
    , m_pParamWidget(new ParamWidget(m_pControlWidget))
    , m_pViewWidget(new ViewWidget(m_pControlWidget))
    , m_pErrorInfoLabel(new QLabel(""))
{
    ui->setupUi(this);
    // 完成界面布局：三个子界面塞进 .ui 的三个占位 QWidget
    ui->ControlWidget->layout()->addWidget(m_pControlWidget);
    ui->ParamWidget->layout()->addWidget(m_pParamWidget);
    ui->ViewWidget->layout()->addWidget(m_pViewWidget);
    ui->statusbar->addWidget(m_pErrorInfoLabel);
    this->resize(1000, 600);

    // 创建信号槽：三路错误信息汇聚到状态栏 + 弹窗
    connect(m_pControlWidget, &ControlWidget::SigUpdateErrorInfo, this, &MainWindow::OnUpdateErrorInfo);
    connect(m_pParamWidget, &ParamWidget::SigUpdateErrorInfo, this, &MainWindow::OnUpdateErrorInfo);
    connect(m_pViewWidget, &ViewWidget::SigUpdateErrorInfo, this, &MainWindow::OnUpdateErrorInfo);

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
        QMessageBox::critical(this, "Error", strErrorInfo, QMessageBox::Close);
    }
}
