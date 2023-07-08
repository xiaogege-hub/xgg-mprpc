aux_source_directory(. SRC_LIST)
#默认编译成动态库
add_library(mprpc ${SRC_LIST})  
# add_library(mprpc SHARED ${SRC_LIST})动态库编译方法
# 指定可执行文件链接时需要依赖的库文件 muduo库的顺序很重要
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)