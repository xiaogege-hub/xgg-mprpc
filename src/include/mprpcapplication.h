#pragma once
#include "config.h"
#include "rpcchannel.h"
#include "rpccontroller.h"

// mprpc框架的基础类 负责框架的一些初始化操作
// 设计成单例类
class MprpcApplication {
 public:
  static void Init(int argc, char **argv);
  static MprpcApplication *GetInstance();
  static MprpcConfig *GetConfig();

 private:
  MprpcApplication() {}
  MprpcApplication(const MprpcApplication &) = delete;
  MprpcApplication(MprpcApplication &&) = delete;

 private:
  // 静态方法中无法访问非静态的成员变量，所以定义成static
  // 静态成员必须类内声明，类外初始化
  static MprpcConfig m_config;
};