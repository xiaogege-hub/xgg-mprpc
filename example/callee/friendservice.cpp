#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "logger.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class FriendService : public xgg::FriendServiceRpc {
 public:
  std::vector<std::string> GetFriendList(uint32_t userid) {
    std::cout << "doing local service : GetFriendList " << std::endl;
    std::vector<std::string> vec;
    vec.push_back("li si");
    vec.push_back("wang wu");
    return vec;
  }

  // 重写基类方法
  void GetFriendList(::google::protobuf::RpcController *controller,
                     const ::xgg::GetFriendListRequest *request,
                     ::xgg::GetFriendListResponse *response,
                     ::google::protobuf::Closure *done) override {
    // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
    uint32_t userid = request->userid();

    // 做本地业务
    std::vector<std::string> friendList = GetFriendList(userid);

    // 把响应写入 包括错误码、错误消息、返回值
    xgg::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg(" ");
    for (std::string &name : friendList) {
      std::string *p = response->add_friends();
      *p = name;
    }

    // 执行回调操作  执行响应对象数据的序列化和网络发送（都是由框架来完成的）
    done->Run();
  }
};

int main(int argc, char **argv) {
  // 测试日志输出是否正确
  //LOG_INFO("first log message!");
  //LOG_ERROR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

  // 调用框架的初始化操作
  MprpcApplication::Init(argc, argv);

  // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
  RpcProvider provider;
  provider.NotifyService(new FriendService());

  // 启动一个rpc服务节点。Run以后，进程进入阻塞状态，等待远程的rpc调用请求
  provider.Run();

  return 0;
}
