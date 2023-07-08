#include <iostream>
#include "mprpcapplication.h"
#include "rpcchannel.h"
#include "user.pb.h"

int main(int argc, char **argv) {
  // 整个程序启动后，想使用mprpc框架来享受rpc服务调用，一定需要先使用框架的初始化函数(只初始化一次)
  MprpcApplication::Init(argc, argv);

  // 演示调用远程发布的rpc方法Login
  // 1.构造UserServiceRpc_Stub对象
  xgg::UserServiceRpc_Stub stub(new MprpcChannel());
  // 2.rpc方法的请求参数
  xgg::LoginRequest request;
  request.set_name("zhangsan");
  request.set_pwd("123456");
  // 3.rpc方法的响应
  xgg::LoginResponse response;
  // 4.以同步的方式发起rpc调用请求 其实在调用MprpcChannel::callmethod
  stub.Login(nullptr, &request, &response, nullptr);

  // 5.一次rpc调用完成，读调用的结果
  if (response.result().errcode() == 0) {
    std::cout << "rpc login response success: " << response.success()
              << std::endl;
  } else {
    std::cout << "rpc login respons error: " << response.result().errmsg()
              << std::endl;
  }

  // ===============================================
  // 演示调用远程发布的rpc方法Register
  xgg::RegisterRequest request_2;
  request_2.set_id(1111);
  request_2.set_name("mprpc");
  request_2.set_pwd("666");
  xgg::RegisterResponse response_2;

  // 以同步的方式发起rpc调用请求，等待返回结果
  stub.Register(nullptr, &request_2, &response_2, nullptr);

  // 一次rpc调用完成，读调用的结果
  if (response_2.result().errcode() == 0) {
    std::cout << "rpc register response success: " << response_2.success()
              << std::endl;
  } else {
    std::cout << "rpc register respons error: " << response_2.result().errmsg()
              << std::endl;
  }

  return 0;
}