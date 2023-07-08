#set(SRC_LIST calluserservice.cpp ../user.pb.cc)
set(SRC_LIST callfriendservice.cpp ../friend.pb.cc)

# 指定生成可执行文件
add_executable(consumer ${SRC_LIST})
# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(consumer mprpc protobuf)