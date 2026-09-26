#ifndef CAMERAPARAMMODEL_H
#define CAMERAPARAMMODEL_H
/**************************************************
 * CameraParamModel参数模型类，该类与CameraParamDelegate、CameraParamItem配合使用
 * 三者共同组成Model-View-Delegate模型(MVD)，不了解该模型可自行百度。
 *
 **************************************************/

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>

class CameraParamItem;
class CameraParam;
class CameraParamModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum ColType {
        NAME = 0,
        VALUE
    };

    enum ItemRoles {
        // 从 Qt::UserRole 开始定义
        ParamRole = Qt::UserRole + 1, // 用于存储CameraParam
        ParamDescriptionRole = Qt::UserRole + 2, // 用于获取参数描述信息
    };

    CameraParamModel(const QStringList& headers, QObject* parent = nullptr);
    ~CameraParamModel();

    // 【1】自定义接口组
    // 添加模型数据
    void addCameraParam(CameraParam& param);
    void clear(); // 清空数据

    // 【2】只读模型接口组
    // 模型数组接口组
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // 【3】可编辑模型接口组
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

signals:
    void SigValueChanged(const QModelIndex& index);

protected:
    CameraParamItem* getItem(const QModelIndex& index) const;

private:
    CameraParamItem* m_pRootItem;
    QStringList m_headers;
    QMap<QString, CameraParamItem*> m_groups;
};

#endif // CAMERAPARAMMODEL_H
