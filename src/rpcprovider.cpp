#include "rpcprovider.h"

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口。
void RpcProvider::NotifyService(google::protobuf::Service *service) {
  // 获取服务对象的描述信息
  const google::protobuf::ServiceDescriptor *pserviceDesc =
      service->GetDescriptor();
  // 1.获取服务的名字
  std::string service_name = pserviceDesc->name();

  // 获取服务对象service方法的数量
  int methodCnt = pserviceDesc->method_count();

  LOG_INFO("service_name:%s", service_name.c_str());

  ServiceInfo service_info;
  for (int i = 0; i < methodCnt; ++i) {
    // 2.获取了服务对象指定下标的服务方法的描述（抽象描述）
    const google::protobuf::MethodDescriptor *pmethodDesc =
        pserviceDesc->method(i);
    std::string method_name = pmethodDesc->name();
    service_info.m_methodMap.insert({method_name, pmethodDesc});

    LOG_INFO("method_name:%s", method_name.c_str());
  }
  service_info.m_service = service;
  m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run() {
  // 读取配置文件rpcserver的信息
  std::string ip =
      MprpcApplication::GetInstance()->GetConfig()->Load("rpcserverip");
  uint16_t port = atoi(MprpcApplication::GetInstance()
                           ->GetConfig()
                           ->Load("rpcserverport")
                           .c_str());
  muduo::net::InetAddress address(ip, port);

  // 创建TcpServer对象  封装在这里是为了不让用户传入过多参数
  muduo::net::TcpServer m_tcpserver(&m_eventLoop, address, "RpcProvider");

  // 绑定连接回调和消息读写回调方法  分离了网络代码和业务代码
  m_tcpserver.setConnectionCallback(
      std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
  m_tcpserver.setMessageCallback(
      std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));

  // 设置muduo库的线程数量 一个io线程 三个工作线程
  m_tcpserver.setThreadNum(4);

  // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
  // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout
  // 时间发送ping消息
  /*ZkClient zkCli;
  zkCli.Start();
  // service_name为永久性节点    method_name为临时性节点
  for (auto &sp : m_serviceMap) {
    // /service_name   /UserServiceRpc
    std::string service_path = "/" + sp.first;
    zkCli.Create(service_path.c_str(), nullptr, 0);
    for (auto &mp : sp.second.m_methodMap) {
      // /service_name/method_name   /UserServiceRpc/Login
      // 存储当前这个rpc服务节点主机的ip和port
      std::string method_path = service_path + "/" + mp.first;
      char method_path_data[128] = {0};
      sprintf(method_path_data, "%s:%d", ip.c_str(), port);
      // ZOO_EPHEMERAL表示znode是一个临时性节点
      zkCli.Create(method_path.c_str(), method_path_data,
                   strlen(method_path_data), ZOO_EPHEMERAL);
    }
  }*/

  LOG_INFO("RpcProvider start service at ip: %s port: %d", ip.c_str(), port);

  // 启动网络服务
  m_tcpserver.start();
  m_eventLoop.loop();
}

// 连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn) {
  if (!conn->connected()) {
    // 和rpc client的连接断开了
    conn->shutdown();
  }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型

传输数据的格式：
header_size(4个字节) + header_str + args_str

定义proto的message类型，进行数据头的序列化和反序列化
数据头(header_str)的格式：service_name method_name args_size

16UserServiceLoginzhangsan123456
10 "10"
std::string   insert和copy方法
*/
// 消息回调 如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer, muduo::Timestamp time) {
  // 0.网络上接收的远程rpc调用请求的字符流  Login args
  std::string recv_buf = buffer->retrieveAllAsString();

  // 1.从字符流中读取前4个字节的内容（读取header_size）
  uint32_t header_size = 0;
  recv_buf.copy((char *)&header_size, 4, 0);

  // 2.根据header_size读取数据头的原始字符流(读取header_str)
  // 反序列化数据，得到rpc请求的详细信息
  std::string header_str = recv_buf.substr(4, header_size);
  mprpc::RpcHeader rpcHeader;
  std::string service_name;
  std::string method_name;
  uint32_t args_size = 0;
  if (rpcHeader.ParseFromString(header_str)) {
    // 数据头header_str反序列化成功
    service_name = rpcHeader.service_name();
    method_name = rpcHeader.method_name();
    args_size = rpcHeader.args_size();
  } else {
    // 数据头header_str反序列化失败
    LOG_ERROR("header_str: %s  parse error!", header_str.c_str());
    return;
  }

  // 3.根据args_size读取rpc方法参数的字符流数据（读取args_str）
  std::string args_str = recv_buf.substr(4 + header_size, args_size);

  // 打印调试信息
  std::cout << "======================================" << std::endl;
  std::cout << "recv_buf " << recv_buf << std::endl;
  std::cout << "header_size " << header_size << std::endl;
  std::cout << "header_str " << header_str << std::endl;
  std::cout << "service_name " << service_name << std::endl;
  std::cout << "method_name " << method_name << std::endl;
  std::cout << "args_size " << args_size << std::endl;
  std::cout << "args_str " << args_str << std::endl;
  std::cout << "======================================" << std::endl;

  // 4.获取service对象和method对象
  auto it = m_serviceMap.find(service_name);
  if (it == m_serviceMap.end()) {
    // 服务不存在
    LOG_ERROR("%s is not exist!", service_name.c_str());
    return;
  }

  auto method_it = it->second.m_methodMap.find(method_name);
  if (method_it == it->second.m_methodMap.end()) {
    // 服务的某个方法不存在
    LOG_ERROR("%s : %s is not exist!", service_name.c_str(),
              method_name.c_str());
    return;
  }

  google::protobuf::Service *service = it->second.m_service;  // 获取service对象
  const google::protobuf::MethodDescriptor *method =
      it->second.m_methodMap[method_name];  // 获取method对象

  // 5.生成rpc方法调用的请求request和响应response参数
  google::protobuf::Message *request =
      service->GetRequestPrototype(method).New();
  if (!request->ParseFromString(args_str)) {
    LOG_ERROR("request parse error, content: %s", args_str.c_str());
    return;
  }
  google::protobuf::Message *response =
      service->GetResponsePrototype(method).New();

  // 6.在框架上，根据远端rpc请求，调用当前rpc节点上发布的方法
  // 6.1给CallMethod方法的调用，绑定一个Closure的回调函数 done
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<RpcProvider,
                                    const muduo::net::TcpConnectionPtr &,
                                    google::protobuf::Message *>(
          this, &RpcProvider::SendRpcResponse, conn, response);
  // 6.2相当于 new UserService().Login(controller, request, response, done)
  service->CallMethod(method, nullptr, request, response, done);
}

// Closure回调，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response) {
  // 1.对响应数据序列化
  std::string response_str;
  if (response->SerializeToString(&response_str)) {
    // 2.序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
    conn->send(response_str);
  } else {
    LOG_ERROR("serialize response_str error!");
  }
  // 3.模拟http的短连接服务，由rpcprovider主动断开连接（区别chatserver保持长连接）
  conn->shutdown();
}