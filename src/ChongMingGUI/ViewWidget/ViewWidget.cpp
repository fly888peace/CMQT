#include "ViewWidget.h"
#include "AcquireImageProcess.h"
#include "CameraInterface/CMCameraMetaInfo.h"
#include "CameraInterface/CMCameraParam.h"
#include "CameraInterface/CameraContext.h"
#include "CameraInterface/CameraError.h"
#include "ControlWidget/ControlWidget.h"
#include "ViewWidget/GraphicsView.h"
#include "ui_ViewWidget.h"

ViewWidget::ViewWidget(ControlWidget* controlWidget, QWidget* parent)
    : QWidget(parent)
    , Listener()
    , ui(new Ui::ViewWidget)
    , m_pViewBox(new GraphicsView())
    , m_pControlWidget(controlWidget)
    , m_pImageProcess(new AcquireImageProcess(this)) // 复刻修正：挂到父子树，源工程裸 new 泄漏
{
    ui->setupUi(this);
    ui->ViewBox_widget->layout()->addWidget(m_pViewBox);

    // 创建信号槽
    connect(m_pImageProcess, &AcquireImageProcess::sigUpdateImage, m_pViewBox, &GraphicsView::SetImage);
    // 取图线程的报错（如连续超时）转发给 MainWindow
    connect(m_pImageProcess, &AcquireImageProcess::sigErrorInfo, this, &ViewWidget::SigUpdateErrorInfo);

    // 注册事件监听
    ListenerManger::Instance()->registerMessage(MESSAGE::CAMERA_CONNECT
            | MESSAGE::CAMERA_DISCONNECT
            | MESSAGE::CAMERA_ENUMRTION
            | MESSAGE::CAMERA_STARTGRAB
            | MESSAGE::CAMERA_STOPTGRAB,
        this);
}

ViewWidget::~ViewWidget()
{
    // 复刻修正：先停取图线程再拆界面（AcquireImageProcess 析构也有兜底，双保险）
    m_pImageProcess->requestStop();
    m_pImageProcess->wait(3000);
    delete ui;
}

void ViewWidget::RespondMessage(int message)
{
    if ((message & MESSAGE::CAMERA_DISCONNECT) == MESSAGE::CAMERA_DISCONNECT) {
        m_pViewBox->Clear();
        setStarGrabbingState(false);
    }
    if ((message & MESSAGE::CAMERA_ENUMRTION) == MESSAGE::CAMERA_ENUMRTION) {
        m_pViewBox->Clear();
        setStarGrabbingState(false);
    }
    if ((message & MESSAGE::CAMERA_STARTGRAB) == MESSAGE::CAMERA_STARTGRAB) {
        setStarGrabbingState(true);
    }
    if ((message & MESSAGE::CAMERA_STOPTGRAB) == MESSAGE::CAMERA_STOPTGRAB) {
        setStarGrabbingState(false);
    }
}

void ViewWidget::on_Grabbing_Button_toggled(bool checked)
{
    Q_UNUSED(checked);
    QString serial = m_pControlWidget->GetCurrentCameraInfo().Serial;
    if (serial.isNull() || serial.isEmpty())
        return;

    bool state { false };
    CameraContext::Instance()->isGrabbing(serial, state);
    if (state == false) // 未拉流，开启拉流
    {
        connect(m_pImageProcess, &AcquireImageProcess::sigUpdateImage, m_pViewBox, &GraphicsView::SetImage);
        CHECK_RETURN(CameraContext::Instance()->startGrabbing(serial));

        // 开启取图线程
        m_pImageProcess->setSerial(serial);
        m_pImageProcess->start();

        // m_pViewBox->SetImage(QImage(":/zhouxuan.jpg"));
        ListenerManger::Instance()->notify(MESSAGE::CAMERA_STARTGRAB);
    } else {
        disconnect(m_pImageProcess, &AcquireImageProcess::sigUpdateImage, m_pViewBox, &GraphicsView::SetImage);

        // 复刻修正（DEV_SPEC 附录 #1）：先置停止标志，再停流（停流会 Stop 队列唤醒
        // 阻塞在 Take 的取图线程），最后 wait 等线程真正退出——
        // 源工程的 quit() 对没有事件循环的 run() 完全无效，wait() 也被注释掉了
        m_pImageProcess->requestStop();
        CHECK_RETURN(CameraContext::Instance()->stopGrabbing(serial));
        m_pImageProcess->wait();

        ListenerManger::Instance()->notify(MESSAGE::CAMERA_STOPTGRAB);
    }
}

void ViewWidget::setStarGrabbingState(bool state)
{
    if (state == true) {
        ui->Grabbing_Button->setStyleSheet("QPushButton{image:url(:/StopGrab.png);}");
    } else {
        ui->Grabbing_Button->setStyleSheet("QPushButton{image:url(:/StratGrab.png);}");
    }
}
