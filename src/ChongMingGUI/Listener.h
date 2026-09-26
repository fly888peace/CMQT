#ifndef LISTENER_H
#define LISTENER_H
/**************************************************
 * Listener为监听者类，在设计模式中，有监听者就有观察者类，
 * 在重明中，因为我们软件功能并没有太过复杂，没有过多的观察者监听者组合。
 * 所以我们只需要一个观察者来统一管理所有的监听事件就可以了。因此我们将观察者
 * Observer改成了ListenerManger，该类是一个单例。
 *
 * 在重明中监听者只有三个：
 * 		# ControlWidget控制界面类
 *		# ParamWidget参数界面类
 *		# ViewWidget视觉窗口类
 * 而事件也只有相机枚举、连接/断连、拉流/停止拉流、相机切换这几种。
 * 因此这个监听者模式规模比较小。通过监听者模式可以让我们避免使用大量的信号槽去处理我们
 * 不同监听者对不同事件的处理。
 *
 * 复刻说明：
 * - 消息枚举是「位掩码」，取值必须是 2 的幂，才能用 A | B 组合注册、用 & 拆包；
 *   （烛照版曾取 1,2,3,4 导致位运算误命中，重明源工程本身取对了，复刻保持。）
 * - 单例改用函数内静态局部变量（Meyers Singleton），修复源工程
 *   `new ListenerManger()` 永不释放的泄漏（DEV_SPEC 附录 #10）。
 **************************************************/

#include <QMap>
#include <QVector>

enum MESSAGE {
    CAMERA_ENUMRTION = 0x00000001, // 相机枚举
    CAMERA_CONNECT = 0x00000002, // 相机连接
    CAMERA_DISCONNECT = 0x00000004, // 相机断连
    CAMERA_STARTGRAB = 0x00000008, // 相机开启拉流
    CAMERA_STOPTGRAB = 0x00000010, // 相机断开拉流
    CAMERA_CAMERASWICH = 0x00000020 // 相机切换
};

class Listener {
public:
    Listener() { };
    virtual ~Listener() { };
    virtual void RespondMessage(int message) = 0;
};

class ListenerManger {
    typedef QMap<int, QVector<Listener*>> mmap; // 类型重定义
public:
    // 获取单例类对象指针
    static ListenerManger* Instance();
    // 事件到来，通知监听者
    void notify(int message);
    // 注册事件
    void registerMessage(int message, Listener* listener);

private:
    ListenerManger() { };
    ~ListenerManger() { };
    QMap<int, QVector<Listener*>> m_messageToLister; // 事件-监听者列表
};

#endif // LISTENER_H
