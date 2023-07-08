#include <iostream>
#include "friend.pb.h"
#include "mprpcapplication.h"

int main(int argc, char **argv) {
  // 整个程序启动后，想使用mprpc框架来享受rpc服务调用，一定需要先使用框架的初始化函数(只初始化一次)
  MprpcApplication::Init(argc, argv);

  // 演示调用远程发布的rpc方法Login
  // 1.构造FriendServiceRpc_Stub对象
  xgg::FriendServiceRpc_Stub stub(new MprpcChannel());
  // 2.rpc方法的请求参数
  xgg::GetFriendListRequest request;
  request.set_userid(1234);
  // 3.rpc方法的响应
  xgg::GetFriendListResponse response;
  // 4.以同步的方式发起rpc调用请求 其实在调用MprpcChannel::callmethod
  MprpcController controller;
  stub.GetFriendList(&controller, &request, &response, nullptr);

  // 5.一次rpc调用完成，读调用的结果
  if (controller.Failed()) {
    std::cout << controller.ErrorText() << std::endl;
  } else {
    if (response.result().errcode() == 0) {
      std::cout << "rpc GetFriendList response success: " << std::endl;
      int size = response.friends_size();
      for (int i = 0; i < size; ++i) {
        std::cout << "index: " << (i + 1) << " name: " << response.friends(i)
                  << std::endl;
      }
    } else {
      std::cout << "rpc GetFriendList response error: "
                << response.result().errmsg() << std::endl;
    }
  }

  return 0;
}