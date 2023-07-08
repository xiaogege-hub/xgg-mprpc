#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// 异步写日志的日志缓冲队列 (线程安全的队列)
template <typename T>
class LockQueue {
 public:
  // 由我们的mprpc框架的工作线程调用 工作线程->LOG_INFO->Push
  void Push(const T &data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
    m_condvariable.notify_one();
  }

  // 由写日志线程调用 Logger中的 writeLogTask线程
  T Pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
      // 日志队列为空，线程进入wait状态
      m_condvariable.wait(lock);
    }

    T data = m_queue.front();
    m_queue.pop();
    return data;
  }

 private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_condvariable;
};