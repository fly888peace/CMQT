#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H
/**************************************************
 * ViewWidget视觉窗口类。
 * 我们将重明的界面划分为三个子界面，也是三个功能的抽象，分别是
 *		# ControlWidget控制界面类
 *		# ParamWidget参数界面类
 *		# ViewWidget视觉窗口类
 *
 * ViewWidget视觉窗口类包含了相机图像显示、相机拉流控制功能。
 * 同时该类不仅是功能的实现，本身也是一个界面类，其包含了一个控制条和一个视觉图像显示窗口。
 * 此外，该类还是一个监听者类，继承自Listener父类，通过注册监听事件从而对一些事件进行监听。
 *
 **************************************************/

#include "Listener.h"
#include <QWidget>

namespace Ui {
class ViewWidget;
}

class GraphicsView;
class ControlWidget;
class AcquireImageProcess;
class ViewWidget : public QWidget, Listener {
    Q_OBJECT

public:
    explicit ViewWidget(ControlWidget* controlWidget, QWidget* parent = nullptr);
    ~ViewWidget();
    void RespondMessage(int message) override;

signals:
    void SigUpdateErrorInfo(QString info);

private slots:
    void on_Grabbing_Button_toggled(bool checked);

private:
    void setStarGrabbingState(bool state);

private:
    Ui::ViewWidget* ui;
    GraphicsView* m_pViewBox;

    // 和ControlWidget建立绑定关系
    ControlWidget* m_pControlWidget;
    // 采图线程
    AcquireImageProcess* m_pImageProcess;
};

#endif // VIEWWIDGET_H
