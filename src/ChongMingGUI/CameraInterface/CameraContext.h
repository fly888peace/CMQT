#ifndef CAMERACONTEXT_H
#define CAMERACONTEXT_H
/**************************************************
 * CameraContext相机环境类，该类可以理解为前端界面和后端相机之间的一个中间层
 * 所有工业相机相关的功能，如相机的连接、拉流、取图、参数设置等，都通过该类来完成交互。
 * 该类是一个单例类，在软件界面的任何地方，都可以通过CameraContext::Instance()获取单例
 * 指针来进行一些相机操作。
 *
 * 其维护了一个最重要的东西就是序列号与相机之间的映射表m_serialCamMap。调用任何函数，都
 * 需要先通过这个表查找序列号对应的相机指针，然后调用指针进行对应操作。
 * 指针是CameraInterface抽象接口类的指针，但可能指向的却是HikCamera这些子类，所以这就是
 * C++的多态。利用多态我们可以继承任何不同品牌的相机SDK，只需要实现类似HikCamera这种子类即可。
 *
 * 复刻说明：
 * - 单例仅被 UI 线程使用，Instance() 不做加锁（与源工程一致，语义上够用）；
 * - 析构时逐一 delete 相机对象，修复源工程「只停流断连、不释放相机」的泄漏（DEV_SPEC 附录 #10）。
 **************************************************/

#include "CameraError.h"
#include <QImage>
#include <QMap>
#include <QString>
#include <QVector>
#include <map>

// 宏定义，用于检查CameraContext接口的返回值
// 注意：只能在「继承 QObject 且定义了 SigUpdateErrorInfo(QString) 信号」的类成员函数里使用，
// 它会把错误翻译成英文串并通过信号抛给 MainWindow 弹窗。
#define CHECK_RETURN(Ret)                    \
    if (Ret != CHONGMING_OK) {               \
        QString error = getErrorInfoEn(Ret); \
        emit SigUpdateErrorInfo(error);      \
        return;                              \
    }

struct CameraMetaInfo;
class CameraParam;
class CameraInterface;
class CameraImageQueue;
class CameraContext {
public:
    // 获取单例指针
    static CameraContext* Instance();
    // 释放单例指针，单例模式必须要有Release，且退出软件时需要手动调用释放
    static void Release();
    //[1]相机接口组
    // 枚举相机
    uint32_t EnumerationCamera(QVector<CameraMetaInfo>& cameraInfos);
    // 获取相机参数列表
    uint32_t getParamList(const QString serial, QVector<CameraParam>& paramList);
    // 判断相机是否连接
    uint32_t isConnect(const QString serial, bool& state);
    // 判断相机是否拉流
    uint32_t isGrabbing(const QString serial, bool& state);
    // 连接相机
    uint32_t connect(const QString serial);
    // 断开连接
    uint32_t disconnect(const QString serial);
    // 开启拉流
    uint32_t startGrabbing(const QString serial);
    // 停止拉流
    uint32_t stopGrabbing(const QString serial);
    // 加载配置文件
    uint32_t loadConfig(const QString serial, const QString path);
    // 保存配置文件
    uint32_t saveConfig(const QString serial, const QString path);
    // 保存配置文件
    QString getConfigFormat(const QString serial);
    // 读取参数
    uint32_t readParam(const QString serial, CameraParam& param);
    // 写入参数
    uint32_t writeParam(const QString serial, CameraParam& param);
    // 获取图像
    uint32_t getImageLast(const QString serial, QImage& qImage);

private:
    // 单例模式需要将构造与析构函数置为private
    CameraContext();
    ~CameraContext();
    // 清理映射表：停流、断连、释放并 delete 所有相机对象
    void clearAllCameras();

private:
    static CameraContext* m_pContext; // 单例指针
    QMap<QString, CameraInterface*> m_serialCamMap; // 序列号-相机指针
};

#endif // CAMERACONTEXT_H
