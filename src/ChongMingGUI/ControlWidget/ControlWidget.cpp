#include "ControlWidget.h"
#include "CameraInterface/CMCameraMetaInfo.h"
#include "CameraInterface/CameraContext.h"
#include "LoadingDialog/LoadingDialog.h"
#include "ui_ControlWidget.h"
#include <QFileDialog>
#include <algorithm>
#include <iostream>

ControlWidget::ControlWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ControlWidget)
    , Listener()
    , m_lastCameraIndex(-1)
{
    ui->setupUi(this);

    // 注册事件监听（六个消息按位或组合，一次注册）
    // 复刻后 ListenerManger 已支持 CAMERA_CAMERASWICH 分支，这里的注册不再落空
    ListenerManger::Instance()->registerMessage(MESSAGE::CAMERA_CONNECT
            | MESSAGE::CAMERA_DISCONNECT
            | MESSAGE::CAMERA_ENUMRTION
            | MESSAGE::CAMERA_STARTGRAB
            | MESSAGE::CAMERA_STOPTGRAB
            | MESSAGE::CAMERA_CAMERASWICH,
        this);
}

ControlWidget::~ControlWidget()
{
    CameraContext::Release();
    delete ui;
}

// 枚举相机槽函数
// 点击该按钮时进行相机枚举，并刷新相机列表,同时发送枚举监听事件
void ControlWidget::on_Enumeration_Button_clicked()
{
    LoadingDialog::Loading();
    // 枚举相机
    QVector<CameraMetaInfo> cameras;
    CameraContext::Instance()->EnumerationCamera(cameras);

    // 重新赋值相机信息列表，并更新UI界面
    ui->Camera_listWidget->clear();
    m_cameraMetaInfos.clear();
    m_cameraMetaInfos = cameras;
    for (CameraMetaInfo cameraInfo : m_cameraMetaInfos) {
        ui->Camera_listWidget->addItem(cameraInfo.UserDefineID + "(" + cameraInfo.Serial + ")");
    }

    // 发送监听事件
    ListenerManger::Instance()->notify(MESSAGE::CAMERA_ENUMRTION);
    LoadingDialog::HideLoading();
}

// 导出相机参数槽函数
// 点击该按钮时，导出当前所选相机的参数配置文件
void ControlWidget::on_SaveConfig_Button_clicked()
{
    QString serial = GetCurrentCameraInfo().Serial;
    QString format = CameraContext::Instance()->getConfigFormat(serial);
    QString filter = tr("config files(*.%1)").arg(format);

    QString filePath = QFileDialog::getSaveFileName(this, "Save Config",
        "", filter);
    if (filePath.isEmpty())
        return;

    CHECK_RETURN(CameraContext::Instance()->saveConfig(serial, filePath));
}

// 导入相机参数槽函数
// 点击该按钮时，导入当前选中相机的参数配置文件
void ControlWidget::on_LoadConfig_Button_clicked()
{
    QString serial = GetCurrentCameraInfo().Serial;
    QString format = CameraContext::Instance()->getConfigFormat(serial);
    QString filter = tr("config files(*.%1)").arg(format);

    QString filePath = QFileDialog::getOpenFileName(this, "Load Config",
        "", filter);
    if (filePath.isEmpty())
        return;

    CHECK_RETURN(CameraContext::Instance()->loadConfig(serial, filePath));
}

// 切换相机槽函数
// 当相机列表中所选相机发生变化时，进入该槽函数
void ControlWidget::on_Camera_listWidget_currentRowChanged(int currentRow)
{
    if (currentRow == -1)
        return;

    // 处理之前的相机对象,将旧相机断连
    CameraMetaInfo lastCameraInfo = GetCameraInfo(m_lastCameraIndex);
    CameraContext::Instance()->stopGrabbing(lastCameraInfo.Serial);
    CameraContext::Instance()->disconnect(lastCameraInfo.Serial);
    ListenerManger::Instance()->notify(MESSAGE::CAMERA_DISCONNECT);
    m_lastCameraIndex = currentRow;

    // 处理当前的相机对象，正常情况下相机应该处于断连状态
    CameraMetaInfo currentCameraInfo = GetCurrentCameraInfo();
    bool connectState;
    CameraContext::Instance()->isConnect(currentCameraInfo.Serial, connectState);
    if (connectState == true) {
        ui->Connect_Button->setStyleSheet("QPushButton{image:url(:/DisConnect.png);}");
        ListenerManger::Instance()->notify(MESSAGE::CAMERA_CONNECT);
    } else {
        ui->Connect_Button->setStyleSheet("QPushButton{image:url(:/Connect.png);}");
        ListenerManger::Instance()->notify(MESSAGE::CAMERA_DISCONNECT);
    }
    // 复刻修正（DEV_SPEC 附录 #9）：补发相机切换事件，
    // 源工程定义了 CAMERA_CAMERASWICH 却从不 notify，是死事件
    ListenerManger::Instance()->notify(MESSAGE::CAMERA_CAMERASWICH);
}

// 连接/断连槽函数
// 当切换相机的连接状态时，会进入该槽函数
void ControlWidget::on_Connect_Button_toggled(bool checked)
{
    Q_UNUSED(checked)

    CameraMetaInfo currentCameraInfo = GetCurrentCameraInfo();
    QString serial = currentCameraInfo.Serial;
    if (serial.isNull() || serial.isEmpty()) // 如果当前相机信息为空
    {
        ui->Connect_Button->setStyleSheet("QPushButton{image:url(:/Connect.png);}");
        return;
    }

    bool connectState;
    CameraContext::Instance()->isConnect(serial, connectState);
    if (connectState == true) {
        CHECK_RETURN(CameraContext::Instance()->stopGrabbing(serial));
        CHECK_RETURN(CameraContext::Instance()->disconnect(serial));
        ui->Connect_Button->setStyleSheet("QPushButton{image:url(:/Connect.png);}");
        ListenerManger::Instance()->notify(MESSAGE::CAMERA_DISCONNECT);
    } else {
        CHECK_RETURN(CameraContext::Instance()->connect(serial));
        ui->Connect_Button->setStyleSheet("QPushButton{image:url(:/DisConnect.png);}");
        ListenerManger::Instance()->notify(MESSAGE::CAMERA_CONNECT);
    }
}

// 获取当前所选中的相机，如果当前未选择相机则返回空对象
CameraMetaInfo ControlWidget::GetCurrentCameraInfo()
{
    int index = ui->Camera_listWidget->currentRow();
    if (index == -1 || index >= m_cameraMetaInfos.size()) {
        return CameraMetaInfo();
    } else {
        return m_cameraMetaInfos.at(index);
    }
}

CameraMetaInfo ControlWidget::GetCameraInfo(int index)
{
    if (index == -1 || index >= m_cameraMetaInfos.size()) {
        return CameraMetaInfo();
    } else {
        return m_cameraMetaInfos.at(index);
    }
}

void ControlWidget::RespondMessage(int message)
{
    if ((message & MESSAGE::CAMERA_ENUMRTION) == MESSAGE::CAMERA_ENUMRTION) {
        on_Connect_Button_toggled(true);
    }
}
