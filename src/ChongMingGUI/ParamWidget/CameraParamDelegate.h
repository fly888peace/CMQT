#ifndef CAMERAPARAMDELEGATE_H
#define CAMERAPARAMDELEGATE_H
/**************************************************
 * CameraParamDelegate模型代理类，该类与CameraParamItem、CameraParamModel配合使用
 * 三者共同组成Model-View-Delegate模型(MVD)，不了解该模型可自行百度。
 *
 * 该代理类实现了对相机参数的代理，包括了对相机六种参数类型的处理：
 * Int、Double、Bool、Enum、String、Cmd
 *
 **************************************************/

#include "CameraInterface/CMCameraParam.h"
#include <QModelIndex>
#include <QPainter>
#include <QStyledItemDelegate>

class CameraParamDelegate : public QStyledItemDelegate {
public:
    CameraParamDelegate(QObject* parent = nullptr);
    virtual ~CameraParamDelegate();
    // 创建编辑器
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    // 设置编辑器数据
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    // 设置模型数据
    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    // 更新编辑器样式
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    // 绘制样式
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    // 设置默认尺寸
    QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

protected slots:
    void onValueChanged(const CameraParam& param, const QModelIndex& index);
};

#endif // CAMERAPARAMDELEGATE_H
