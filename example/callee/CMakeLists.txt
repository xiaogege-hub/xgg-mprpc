#set(SRC_LIST userservice.cpp ../user.pb.cc) 
set(SRC_LIST friendservice.cpp ../friend.pb.cc) 

# 指定生成可执行文件
add_executable(provider ${SRC_LIST})
# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(provider mprpc protobuf)