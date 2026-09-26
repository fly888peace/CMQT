#include "LoadingDialog.h"
#include "ui_LoadingDialog.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>

LoadingDialog* s_LoadingWidget = nullptr;

LoadingDialog::LoadingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoadingDialog)
{
    ui->setupUi(this);
    // 如果需要显示任务栏对话框则删除Qt::Tool
    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    this->setFixedSize(250, 180);

    // 加载Loading动画
    m_pLoadingMovie = new QMovie(":/loading.gif");
    m_pLoadingMovie->setScaledSize(QSize(250, 180));
    ui->Gif_label->setMovie(m_pLoadingMovie);
    m_pLoadingMovie->start();
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}

void LoadingDialog::Loading(QWidget* parent)
{
    if (nullptr == s_LoadingWidget) {
        s_LoadingWidget = new LoadingDialog(parent);
    }
    // 设置界面为模态窗口
    s_LoadingWidget->setModal(true);
    s_LoadingWidget->show();
    QApplication::processEvents();
}

void LoadingDialog::HideLoading()
{
    if (s_LoadingWidget) {
        s_LoadingWidget->close();
        delete s_LoadingWidget;
        s_LoadingWidget = nullptr;
    }
}

void LoadingDialog::on_Close_Button_clicked()
{
    this->close();
}
