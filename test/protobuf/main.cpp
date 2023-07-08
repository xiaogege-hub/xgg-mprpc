#include <iostream>
#include <string>
#include "test.pb.h"
using namespace xgg;

// 利用protobuf实现对数据序列化和反序列化

// GetFriendListResponse对象的序列化和反序列化
int main() {
  GetFriendListResponse rsp;
  ResultCode *rc = rsp.mutable_result();  // 获取他成员对象的指针
  rc->set_errcode(0);
  User *user1 = rsp.add_friend_list();
  user1->set_name("zhangsan");
  user1->set_age(20);
  user1->set_sex(User::MAN);
  User *user2 = rsp.add_friend_list();
  user2->set_name("lisi");
  user2->set_age(19);
  user2->set_sex(User::WOMAN);

  std::cout << rsp.friend_list_size() << std::endl;

  return 0;
}

// LoginResponse对象的序列化和反序列化
int main2() {
  LoginResponse rsp;
  ResultCode *rc = rsp.mutable_result();  // 获取他成员对象的指针
  rc->set_errcode(1);
  rc->set_errmsg("登录处理失败了");
  rsp.set_success(false);

  // 对象数据序列化=》char *
  std::string send_str;
  if (rsp.SerializeToString(&send_str)) {
    std::cout << send_str << std::endl;
  }

  //------------------------------------------------
  // 从send_str反序列化一个login请求对象
  LoginResponse rspB;
  if (rspB.ParseFromString(send_str)) {
    // 说明成功从stirng 序列化到 对象中
    std::cout << rspB.result().errcode() << std::endl;
    std::cout << rspB.result().errmsg() << std::endl;
    std::cout << rspB.success() << std::endl;
  }
  return 0;
}

// LoginRequest对象的序列化和反序列化
int main1() {
  // 封装了login请求对象的数据
  LoginRequest req;
  req.set_name("zhangsan");
  req.set_pwd("123456");

  // 对象数据序列化=》char *
  std::string send_str;
  if (req.SerializeToString(&send_str)) {
    std::cout << send_str << std::endl;
  }

  //------------------------------------------------
  // 从send_str反序列化一个login请求对象
  LoginRequest reqB;
  if (reqB.ParseFromString(send_str)) {
    // 说明成功从stirng 序列化到 对象中
    std::cout << reqB.name() << std::endl;
    std::cout << reqB.pwd() << std::endl;
  }
  return 0;
}