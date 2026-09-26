#ifndef PARAMWIDGET_H
#define PARAMWIDGET_H
/**************************************************
 * ParamWidget参数显示类。
 * 我们将重明的界面划分为三个子界面，也是三个功能的抽象，分别是
 *		# ControlWidget控制界面类
 *		# ParamWidget参数界面类
 *		# ViewWidget视觉窗口类
 *
 * ParamWidget参数显示类包含了对相机参数的读取、写入、显示等功能。
 * 同时该类不仅是功能的实现，本身也是一个界面类，其包含了一个参数注解界面和一个参数列表。
 * 此外，该类还是一个监听者类，继承自Listener父类，通过注册监听事件从而对一些事件进行监听。
 *
 **************************************************/

#include "Listener.h"
#include <QTextBrowser>
#include <QTreeView>
#include <QVector>
#include <QWidget>

namespace Ui {
class ParamWidget;
}

class CameraParamDelegate;
class CameraParamModel;
class ControlWidget;
class CameraParam;
class ParamWidget : public QWidget, Listener {
    Q_OBJECT

public:
    // ParamWidget(QWidget *parent = nullptr);
    explicit ParamWidget(ControlWidget* controlWidget, QWidget* parent = nullptr);
    ~ParamWidget();

    void initParamWidget(QVector<CameraParam> paramList);
    void clearParamWidget();
    void writeCameraParam(const QModelIndex& index);
    void RespondMessage(int message) override;

signals:
    void SigUpdateErrorInfo(QString info);

public slots:
    void OnUpdataSelection(const QItemSelection& selected, const QItemSelection& deselected);

private slots:
    void on_Refresh_Button_clicked();

private:
    Ui::ParamWidget* ui;
    QTreeView* m_pParamTreeView;
    QTextBrowser* m_pParamDescript;
    CameraParamModel* m_pModel;
    CameraParamDelegate* m_pCameraParamDelegate;
    QItemSelectionModel* m_pSelectionModel;

    // 和ControlWidget建立绑定关系
    ControlWidget* m_pControlWidget;
};

#endif // PARAMWIDGET_H
