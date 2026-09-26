#include "ImageItem.h"
#include <QGraphicsSceneHoverEvent>
/**************************************************
 * 图像元素
 * 其实和roi元素是同一类东西，都是在视觉窗口内显示的元素item
 *
 **************************************************/

ImageItem::ImageItem(QWidget* parent)
    : QGraphicsPixmapItem(nullptr)
{
    setAcceptHoverEvents(true);
}

void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    QPointF mousePosition = event->pos();
    int R, G, B;
    int x, y;
    x = mousePosition.x();
    y = mousePosition.y();
    if (mousePosition.x() < 0) {
        x = 0;
    }
    if (mousePosition.y() < 0) {
        y = 0;
    }
    // 复刻修正：坐标右/下越界也要钳制，源工程只钳了负值，
    // 拖出图像右下边缘后 pixelColor 越界取的是未定义数据
    if (x >= pixmap().width() || y >= pixmap().height()) {
        return;
    }
    pixmap().toImage().pixelColor(x, y).getRgb(&R, &G, &B);
    QString InfoVal = QString(" W:%1,H:%2 | X:%3,Y:%4 | R:%5,G:%6,B:%7")
                          .arg(QString::number(w))
                          .arg(QString::number(h))
                          .arg(QString::number(x))
                          .arg(QString::number(y))
                          .arg(QString::number(R))
                          .arg(QString::number(G))
                          .arg(QString::number(B));
    emit RGBValue(InfoVal);
}
