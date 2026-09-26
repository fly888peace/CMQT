#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H
/**************************************************
 * ControlWidget控制界面类。
 * 我们将重明的界面划分为三个子界面，也是三个功能的抽象，分别是
 *		# ControlWidget控制界面类
 *		# ParamWidget参数界面类
 *		# ViewWidget视觉窗口类
 *
 * ControlWidget控制界面类包含了对相机枚举、相机连接、相机配置文件导入导出
 * 等功能的控制。是对相机这些行为的抽象。
 * 同时该类不仅是功能的实现，本身也是一个界面类，其包含了一个控制条和一个相机列表。
 * 此外，该类还是一个监听者类，继承自Listener父类，通过注册监听事件从而对一些事件进行监听。
 *
 **************************************************/

#include "Listener.h"
#include <QWidget>

namespace Ui {
class ControlWidget;
}

struct CameraMetaInfo;
class ControlWidget : public QWidget, Listener {
    Q_OBJECT

public:
    explicit ControlWidget(QWidget* parent = nullptr);
    ~ControlWidget();
    // 响应监听事件
    void RespondMessage(int message) override;

    // 辅助函数
    CameraMetaInfo GetCurrentCameraInfo();
    CameraMetaInfo GetCameraInfo(int index);

signals:
    // 更新报错信息
    void SigUpdateErrorInfo(QString info);

private slots: // 页面按钮的信号槽
               // 枚举按钮，重新枚举相机，刷新枚举相机列表
    void on_Enumeration_Button_clicked();
    // 保存配置文件，导出相机的配置文件
    void on_SaveConfig_Button_clicked();
    // 导入相机的配置文件
    void on_LoadConfig_Button_clicked();
    // 响应相机列表选中的相机切换
    void on_Camera_listWidget_currentRowChanged(int currentRow);
    // 连接或者断连当前选中的相机
    void on_Connect_Button_toggled(bool checked);

private:
    Ui::ControlWidget* ui;
    int m_lastCameraIndex;

    QVector<CameraMetaInfo> m_cameraMetaInfos;
};

#endif // CONTROLWIDGET_H
