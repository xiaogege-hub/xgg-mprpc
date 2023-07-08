#include "logger.h"
#include <time.h>
#include <iostream>

// 获取日志的单例
Logger *Logger::GetInstance() {
  static Logger logger;
  return &logger;
}

Logger::Logger() {
  // 启动专门的写日志线程
  std::thread writeLogTask([&]() {
    for (;;) {
      // 获取当前的日期，然后取日志信息，写入响应的日志文件当中 a+
      // 1.获取当前的年月日
      time_t now = time(nullptr);
      tm *now_tm = localtime(&now);

      // 2.根据年月日信息，得到file_name
      char file_name[128];
      sprintf(file_name, "%d-%d-%d-log.txt", now_tm->tm_year + 1900,
              now_tm->tm_mon + 1, now_tm->tm_mday);

      // 3.根据file_name，打开/创建txt文件
      FILE *pf = fopen(file_name, "a+");
      if (pf == nullptr) {
        std::cout << "logger file : " << file_name << " open error!"
                  << std::endl;
        exit(1);
      }

      // 4.得到日志缓冲队列中的消息
      std::string msg = m_lockQueue.Pop();

      // 5.获取当前的时分秒
      char time_buf[128] = {0};
      sprintf(time_buf, "%d:%d:%d =>[%s] ", now_tm->tm_hour, now_tm->tm_min,
              now_tm->tm_sec, (m_loglevel == INFO ? "info" : "error"));

      // 6.为消息添加上时分秒信息
      msg.insert(0, time_buf);
      msg.append("\n");

      // 7.写入到文件中
      fputs(msg.c_str(), pf);
      fclose(pf);
    }
  });
  // 设置分离线程，守护线程
  writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level) { m_loglevel = level; }

// 写日志 把日志信息写入lockqueue缓冲区中
void Logger::Log(std::string msg) { m_lockQueue.Push(msg); }