#ifndef IMAGECONVER_H
#define IMAGECONVER_H
/**************************************************
 * cv::Mat与QImage之间的转换函数。
 * 在 Qt 程序中使用 OpenCV 时有用。
 *
 * 复刻说明（相对源工程的有意差异）：
 * - `static` 改 `inline`：两者都能避免多重定义，但 inline 让编译器只保留一份实体，
 *   语义上更贴合「头文件共享函数」的意图（烛照版曾因非 inline 自由函数踩过 ODR 坑）。
 * - 去掉函数体内的 qDebug 打印：转换函数在取图热路径上每帧调用，日志噪音会刷屏。
 **************************************************/
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <QtCore/QDebug>
#include <QtGui/QImage>

namespace ImageConver {
// 将OpenCV的cv::Mat类型图像转换为QImage类型
//@param mat 待转换的图像，支持 CV_8UC1、CV_8UC3、CV_8UC4 三种OpenCV 的数据类型
//@param clone true 表示与 Mat 不共享内存，更改生成的 mat 不会影响原始图像，false 则会与 mat 共享内存
//@param rb_swap 只针对 CV_8UC3 格式，如果 true 则会调换 R 与 B RGB->BGR，如果共享内存的话原始图像也会发生变化
//@return 转换后的 QImage 图像
inline QImage cvMat2QImage(const cv::Mat& mat, bool clone = true, bool rb_swap = true)
{
    const uchar* pSrc = (const uchar*)mat.data;
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1) {
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        if (clone)
            return image.copy();
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if (mat.type() == CV_8UC3) {
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        if (clone) {
            if (rb_swap)
                return image.rgbSwapped();
            return image.copy();
        } else {
            if (rb_swap) {
                cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
            }
            return image;
        }

    } else if (mat.type() == CV_8UC4) {
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        if (clone)
            return image.copy();
        return image;
    } else {
        qWarning() << "ERROR: Mat could not be converted to QImage, type =" << mat.type();
        return QImage();
    }
}

//@brief 将QImage的类型图像转换为cv::Mat类型
//@param image 待转换的图像，支持 Format_Indexed8/Format_Grayscale、24 位彩色、32 位彩色格式，
//@param clone true 表示与 QImage 不共享内存，更改生成的 mat 不会影响原始图像，false 则会与 QImage 共享内存
//@param rg_swap 只针对 RGB888 格式，如果 true 则会调换 R 与 B RGB->BGR，如果共享内存的话原始图像也会发生变化
//@return 转换后的 cv::Mat 图像
inline cv::Mat QImage2cvMat(QImage& image, bool clone = true, bool rb_swap = true)
{
    cv::Mat mat;
    switch (image.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        if (clone)
            mat = mat.clone();
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        if (clone)
            mat = mat.clone();
        if (rb_swap)
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.bits(), image.bytesPerLine());
        if (clone)
            mat = mat.clone();
        break;
    default:
        qWarning() << "ERROR: QImage could not be converted to Mat, format =" << image.format();
        break;
    }
    return mat;
}
}

#endif // IMAGECONVER_H
