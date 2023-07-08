#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

/*
我们为什么需要zookeeper？
Rpc服务提供端，要在Rpc节点服务启动前，要把它上面发布的服务往zookeeper上注册；
zookeeper在我们项目中作为服务配置中心；
ServiceName作为永久性节点，底下挂method_name作为临时性节点，临时节点上存储该服务所在的ip地址和端口号；
Rpc调用方在去调用服务时，需要知道服务是在哪台主机上运行的，可以在zookeeper上查找得到；
*/
// 封装了zookeeper原生的C API 封装的zk客户端类
class ZkClient {
 public:
  ZkClient();
  ~ZkClient();
  // zkclient启动连接zkserver
  void Start();
  // 在zkserver上根据指定的path创建znode节点
  void Create(const char *path, const char *data, int datalen, int state = 0);
  // 根据参数指定的znode节点路径，获取znode节点的值
  std::string GetData(const char *path);

 private:
  // zk的客户端句柄
  zhandle_t *m_zhandle;
};