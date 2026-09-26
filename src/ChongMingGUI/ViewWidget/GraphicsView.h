#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H
/**************************************************
 * GraphicsView视觉窗口类，该类为视觉窗口的核心代码
 * 通过重写QGraphicsView的鼠标事件等，实现了一个可以拖动图像
 * 放大缩小、双击居中等功能的视觉窗口。
 *
 ***************************************************/

#include "ImageItem.h"
#include <QBoxLayout>
#include <QGraphicsView>
#include <QLabel>
#include <qevent.h>

class GraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    GraphicsView(QWidget* parent = 0);
    ~GraphicsView();
    // 界面初始化
    bool InitWidget();
    // 设置视觉窗口的图像
    void SetImage(const QImage& image);
    // 清空
    void Clear();

protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

public slots:
    // 视图居中显示
    void OnCenter();
    // 视图缩放
    void OnZoom(double scaleFactor);

private:
    // 辅助函数:自适应大小
    void fitFrame();
    void PrepareBackgroundBoard(bool invertColor = false);

private:
    double m_dZoomValue = 1;

    QGraphicsScene* m_pScene; // 场景
    ImageItem* m_pImageItem; // 图像元素
    QWidget* m_pPosInfoWidget; // 视觉窗口左下方，用于显示鼠标位置以及对应位置像素灰度值
    QLabel* m_pPosInfoLabel; // 显示灰度值的标签
    QImage m_qImage; // 视觉窗口所显示的图像
    QPixmap m_qTilePixmap = QPixmap(36, 36); // 背景图片方格
};

#endif // GRAPHICSVIEW_H
