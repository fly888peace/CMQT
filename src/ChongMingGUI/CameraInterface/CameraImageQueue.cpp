#include "CameraImageQueue.h"
#include "CameraError.h"

CameraImageQueue::CameraImageQueue()
{
    m_queueSize = 10;
    m_needStop = false;
    for (int i = 0; i < m_queueSize; i++) {
        freeImageQueue.push(cv::Mat());
    }
}

CameraImageQueue::CameraImageQueue(int maxSize)
{
    m_queueSize = maxSize;
    m_needStop = false;
    for (int i = 0; i < maxSize; i++) {
        freeImageQueue.push(cv::Mat());
    }
}

uint32_t CameraImageQueue::Put(const cv::Mat& m)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    if (freeImageQueue.size() != 0) {
        // 空闲队列有位置：取出回收缓冲，把新图数据真正拷进去（copyTo 尺寸一致时复用原内存）
        // 源工程这里是 temp = m，cv::Mat 赋值是浅拷贝（共享数据），预分配缓冲形同虚设
        cv::Mat temp = freeImageQueue.front();
        freeImageQueue.pop();
        m.copyTo(temp);
        workImageQueue.push(temp);
    } else {
        // 空闲队列没有位置了，也就是工作队列达到了最大长度
        // 此时将工作队列最旧的图弹出替换为新图——生产者（SDK 回调线程）绝不能阻塞
        // 注意：弹出的旧图直接丢弃，深拷贝入队新图，避免与队列外共享数据
        workImageQueue.pop();
        workImageQueue.push(m.clone());
    }
    m_condition.notify_one();
    return CHONGMING_OK;
}

uint32_t CameraImageQueue::Take(cv::Mat& m)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    std::chrono::milliseconds dura(TIME_OUT_MS); // 订一个5秒的时间
    // wait_for最后一个参数是预制条件，调用wait_for的时候，首先就会判断这个条件，
    // 如果这个条件返回false，那么会继续等待，在超时之前，收到了一个notify，
    // 那么它会再次执行这个预制条件来进行判断，超时的时候也还会再此执行这个条件，
    // 条件成立就不会再进行等待
    // 这种可以用在处理队列事件
    auto state = m_condition.wait_for(locker, dura, [this] { return m_needStop || NotEmpty(); });
    // 复刻修正：超时/被唤醒但队列仍空时要区分两种情况，且都不能返回成功码
    if (state == false) {
        return GETIAMGE_TIMEOUT; // 超时：源工程这里返回 CHONGMING_OK，上层拿到空 Mat 还当成功
    }
    if (m_needStop && workImageQueue.empty()) {
        return CAMERA_QUEUE_STOPPED; // 外部调用 Stop() 主动停队列
    }

    // 交付前 clone：消费者拿到的帧与队列内部缓冲彻底脱钩；
    // 队列自有缓冲回收进空闲队列，之后被 Put 复写时不会影响已交付的图（源工程的跨线程覆写隐患）
    cv::Mat frame = workImageQueue.front();
    workImageQueue.pop();
    m = frame.clone();
    freeImageQueue.push(frame);
    return CHONGMING_OK;
}

void CameraImageQueue::Stop()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_needStop = true;
    m_condition.notify_all();
}

void CameraImageQueue::Restart()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_needStop = false;
}

bool CameraImageQueue::Empty()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return workImageQueue.empty();
}

bool CameraImageQueue::Full()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return workImageQueue.size() == m_queueSize;
}

size_t CameraImageQueue::Size()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return workImageQueue.size();
}

bool CameraImageQueue::isFull() const
{
    bool full = workImageQueue.size() >= m_queueSize;
    return full;
}

bool CameraImageQueue::isEmpty() const
{
    bool empty = workImageQueue.empty();
    return empty;
}

bool CameraImageQueue::NotFull() const
{
    bool full = workImageQueue.size() >= m_queueSize;
    return !full;
}

bool CameraImageQueue::NotEmpty() const
{
    bool empty = workImageQueue.empty();
    return !empty;
}
