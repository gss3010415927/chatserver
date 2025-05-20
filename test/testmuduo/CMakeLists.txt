
#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST
    ./muduo_server.cpp
)
#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#aux_source_directory(. SRC_LIST)
#表示生成可执行文件server，由SRC_LIST中的源文件编译而成
add_executable(server ${SRC_LIST})
#表示server需要链接的库文件
target_link_libraries(server muduo_net muduo_base pthread)