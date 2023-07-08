#include "rpcchannel.h"
#include <arpa/inet.h>
#include <error.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>

/*
传输数据的格式：
header_size(4个字节) + header_str（service_name method_name args_size）+
args_str
*/
// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done) {
  // 1.获取service_name和method_name
  const google::protobuf::ServiceDescriptor *service_desc = method->service();
  std::string service_name = service_desc->name();
  std::string method_name = method->name();

  // 2.获取参数的序列化字符串args_str和长度args_size
  uint32_t args_size = 0;
  std::string args_str;
  if (request->SerializeToString(&args_str)) {
    args_size = args_str.size();
  } else {
    controller->SetFailed("serialize request error!");
    return;
  }

  // 3.构造rpc请求的rpcHeader并序列化得到header_str,同时得到header_size
  mprpc::RpcHeader rpcHeader;
  rpcHeader.set_service_name(service_name);
  rpcHeader.set_method_name(method_name);
  rpcHeader.set_args_size(args_size);

  std::string header_str;
  uint32_t header_size = 0;
  if (rpcHeader.SerializeToString(&header_str)) {
    header_size = header_str.size();
  } else {
    controller->SetFailed("serialize rpc header error!");
    return;
  }

  // 4.组织待发送的rpc请求的字符串 利用std::string insert copy
  std::string send_str;
  send_str.insert(0, std::string((char *)(&header_size), 4));  // header_size
  send_str += header_str;                                      // header_str
  send_str += args_str;                                        // args_str

  // 打印调试信息
  std::cout << "======================================" << std::endl;
  std::cout << "send_str " << send_str << std::endl;
  std::cout << "header_size " << header_size << std::endl;
  std::cout << "header_str " << header_str << std::endl;
  std::cout << "service_name " << service_name << std::endl;
  std::cout << "method_name " << method_name << std::endl;
  std::cout << "args_size " << args_size << std::endl;
  std::cout << "args_str " << args_str << std::endl;
  std::cout << "======================================" << std::endl;

  // 5.网络发送。TCP网络编程，完成rpc方法的远程调用 (相当于客户端，不用高并发)
  // 5.1创建socket
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd == -1) {
    char errtxt[512] = {0};
    sprintf(errtxt, "create socket error! errno: %d", errno);
    controller->SetFailed(errtxt);

    close(clientfd);
    return;
  }

  // 5.2 读取配置文件rpcserver的信息
  std::string ip =
      MprpcApplication::GetInstance()->GetConfig()->Load("rpcserverip");
  uint16_t port = atoi(MprpcApplication::GetInstance()
                           ->GetConfig()
                           ->Load("rpcserverport")
                           .c_str());

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

  // 5.3 connect连接rpc服务节点
  if (connect(clientfd, (struct sockaddr *)(&server_addr),
              sizeof(server_addr)) == -1) {
    char errtxt[512] = {0};
    sprintf(errtxt, "connect error! errno: %d", errno);
    controller->SetFailed(errtxt);

    close(clientfd);
    return;
  }

  // 5.4 send网络发送 发送rpc请求
  if (send(clientfd, send_str.c_str(), send_str.size(), 0) == -1) {
    char errtxt[512] = {0};
    sprintf(errtxt, "send error! errno: %d", errno);
    controller->SetFailed(errtxt);

    close(clientfd);
    return;
  }

  // 5.5 recv接收响应结果
  char recv_buf[1024] = {0};
  int recv_size = 0;
  if ((recv_size = recv(clientfd, recv_buf, 1024, 0)) == -1) {
    char errtxt[512] = {0};
    sprintf(errtxt, "recv error! errno: %d", errno);
    controller->SetFailed(errtxt);

    close(clientfd);
    return;
  }

  // 6.将响应结果 反序列化 到 google::protobuf::Message *response中
  if (!response->ParseFromArray(recv_buf, recv_size)) {
    char errtxt[2048] = {0};
    sprintf(errtxt, "parse error! response_str: %s", recv_buf);
    controller->SetFailed(errtxt);

    close(clientfd);
    return;
  }

  close(clientfd);
}
