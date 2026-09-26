#ifndef IMAGEITEM_H
#define IMAGEITEM_H
/**************************************************
 * ImageItem图像元素类。
 * 被包含于视觉窗口中，该类继承自QGraphicsPixmapItem，在原生QGraphicsPixmapItem
 * 的基础上添加了对鼠标移动事件的处理，从而实现了视觉窗口左上角鼠标移动，显示对应图像RGB颜色的功能。
 *
 **************************************************/

#include <QGraphicsPixmapItem>

// 通过鼠标点选获取当前灰度值
class ImageItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    explicit ImageItem(QWidget* parent = nullptr);
signals:
    void RGBValue(QString InfoVal);

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event);

public:
    int w = 0;
    int h = 0;
};

#endif // IMAGEITEM_H
