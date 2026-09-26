#ifndef CAMERAPARAMITEM_H
#define CAMERAPARAMITEM_H
/**************************************************
 * CameraParamItem参数元素类，该类与CameraParamDelegate、CameraParamModel配合使用
 * 三者共同组成Model-View-Delegate模型(MVD)，不了解该模型可自行百度。
 *
 **************************************************/

#include <QVariant>
#include <QVector>

class CameraParamItem {
public:
    explicit CameraParamItem(const QVariant& data, CameraParamItem* parent = nullptr);
    ~CameraParamItem();

    CameraParamItem* child(int number);
    int childCount() const;
    QVariant data() const;
    CameraParamItem* insertChildren(int position);
    CameraParamItem* parent();
    bool removeChildren(int position);
    int childNumber() const;
    bool setData(const QVariant& value);

private:
    QVector<CameraParamItem*> childItems;
    QVariant itemData;
    CameraParamItem* parentItem;
};

#endif // CAMERAPARAMITEM_H
