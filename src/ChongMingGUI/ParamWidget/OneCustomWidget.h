#ifndef ONECUSTOMWIDGET_H
#define ONECUSTOMWIDGET_H
/**************************************************
 * OneCustomWidget是一个基类，我们相机参数有六种不同的类型：Int/Double/Cmd/Bool/String/Enum，
 * 这六种不同的类型，在我们参数面板上所对应的控件也是不同的，例如Int是一个Spinbox控件，
 * String则是LineEdit控件，Cmd命令则是Button按钮控件。
 * 这些不同类型的控件全部都会继承自OneCustomWidget，实现参数面板的可扩展性。
 * 当我们新增一个其他参数类型时（当然其实工业相机固定的就是这六种，并不会有其他类型了），我们可以通过
 * 新的子类化完成扩展。
 *
 **************************************************/

#include "CameraInterface/CMCameraParam.h"
#include <QLayout>
#include <QModelIndex>
#include <QWidget>

class OneCustomWidget : public QWidget {
    Q_OBJECT
public:
    explicit OneCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);
    void InitWidget();

    virtual void setParam(CameraParam& param);
    virtual CameraParam getParam();

signals:
    void sigValueChanged(const CameraParam& param, const QModelIndex& index);

protected:
    virtual void addEditLayout(QHBoxLayout* layout);

protected:
    CameraParam m_param;
    QModelIndex m_index;
};

#endif // ONECUSTOMWIDGET_H
