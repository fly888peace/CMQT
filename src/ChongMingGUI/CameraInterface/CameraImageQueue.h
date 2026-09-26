#ifndef CAMERAIMAGEQUEUE_H
#define CAMERAIMAGEQUEUE_H
/**************************************************
 * CameraImageQueue相机图像队列，基于C++11实现的线程同步队列
 * 队列的成员采用了cv::Mat,用来存储我们的图像，但其实队列的成员我们可以随意设计
 * 例如，我们可以自己设计一个Image类来保存我们的图像，只不过我们采用了
 * opencv自带的Mat
 *
 * 复刻修正（DEV_SPEC 附录 #2 #3）：
 * - Take 超时返回 GETIAMGE_TIMEOUT（源工程返回 CHONGMING_OK，上层无法区分）；
 * - 新增 Stop()/Restart()：置 m_needStop 并 notify_all，让阻塞在 Take 的消费者能退出
 *   （源工程 m_needStop 定义了却从无接口置位，是死代码）；
 * - Put 用 copyTo 把数据真正拷进回收缓冲、Take 交付 clone 后再回收缓冲，
 *   消除 cv::Mat 浅拷贝导致的跨线程图像覆写（源工程 temp = m 只是引用共享）。
 **************************************************/

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>

#define TIME_OUT_MS 5000 // 取图超时时间
#define ImageQueueSize 10 // 图像队列长度宏定义

class CameraImageQueue {
public:
    CameraImageQueue();
    CameraImageQueue(int maxSize);
    // 向图像队列中加入图像
    uint32_t Put(const cv::Mat& m);
    // 从图像队列中取出图像
    uint32_t Take(cv::Mat& m);
    // 停止队列：唤醒所有阻塞在 Take 的消费者（停流/断连时调用）
    void Stop();
    // 复位停止标志：重新开流前调用
    void Restart();
    // 队列是否为空
    bool Empty();
    // 队列是否为满
    bool Full();
    // 队列当前长度
    size_t Size();

private:
    bool isFull() const;
    bool isEmpty() const;
    bool NotFull() const;
    bool NotEmpty() const;

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<cv::Mat> freeImageQueue; // 空闲队列
    std::queue<cv::Mat> workImageQueue; // 工作队列

    uint8_t m_queueSize;
    bool m_needStop;
};

#endif // CAMERAIMAGEQUEUE_H
