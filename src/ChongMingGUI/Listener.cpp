#include "Listener.h"

// Meyers Singleton：C++11 起函数内静态局部变量的初始化是线程安全的，
// 且程序退出时自动析构——替代源工程的 `new ListenerManger()`（永不释放）。
ListenerManger* ListenerManger::Instance()
{
    static ListenerManger s_instance;
    return &s_instance;
}

void ListenerManger::notify(int message)
{
    // 通过键值在map容器中搜索该事件是否在容器中注册
    mmap::iterator iter = m_messageToLister.find(message);
    // 事件存在，通知对应的监听者做出反应（调用该监听者接口）
    if (iter != m_messageToLister.end()) {
        QVector<Listener*>::iterator listener = iter.value().begin();
        while (listener != iter.value().end()) {
            (*listener)->RespondMessage(message); // 调用监听者的函数接口
            listener++;
        }
    } else // 未注册过该事件
    {
        // cout << "no Listener has insterested this meeage" << endl;
    }
}

void ListenerManger::registerMessage(int message, Listener* listener)
{
    auto Register = [&](int singleMessage) -> void {
        mmap::iterator iter = m_messageToLister.find(singleMessage);
        // 没有注册该事件
        if (iter == m_messageToLister.end()) {
            QVector<Listener*> listeners;
            listeners.push_back(listener);
            m_messageToLister[singleMessage] = listeners;
        } else // 有该事件
        {
            // 复刻修正：去重，防止同一监听者对同一消息重复注册导致一次广播响应多次
            if (!iter.value().contains(listener)) {
                iter.value().push_back(listener);
            }
        }
    };

    if ((message & MESSAGE::CAMERA_ENUMRTION) == MESSAGE::CAMERA_ENUMRTION) {
        Register(MESSAGE::CAMERA_ENUMRTION);
    }
    if ((message & MESSAGE::CAMERA_CONNECT) == MESSAGE::CAMERA_CONNECT) {
        Register(MESSAGE::CAMERA_CONNECT);
    }
    if ((message & MESSAGE::CAMERA_DISCONNECT) == MESSAGE::CAMERA_DISCONNECT) {
        Register(MESSAGE::CAMERA_DISCONNECT);
    }
    if ((message & MESSAGE::CAMERA_STARTGRAB) == MESSAGE::CAMERA_STARTGRAB) {
        Register(MESSAGE::CAMERA_STARTGRAB);
    }
    if ((message & MESSAGE::CAMERA_STOPTGRAB) == MESSAGE::CAMERA_STOPTGRAB) {
        Register(MESSAGE::CAMERA_STOPTGRAB);
    }
    // 复刻修正：补上源工程漏掉的 CAMERA_CAMERASWICH 分支（DEV_SPEC 附录 #9），
    // 否则 ControlWidget / ParamWidget 注册相机切换事件永远落空。
    if ((message & MESSAGE::CAMERA_CAMERASWICH) == MESSAGE::CAMERA_CAMERASWICH) {
        Register(MESSAGE::CAMERA_CAMERASWICH);
    }
}
