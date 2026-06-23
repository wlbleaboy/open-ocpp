# 设置构建类型为调试模式
set(CMAKE_BUILD_TYPE Debug)

# 添加编译选项，启用调试信息
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 添加链接选项，包含调试信息
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

# Subdirectories
add_subdirectory(messages)
add_subdirectory(ocpp16)
add_subdirectory(ocpp20)
add_subdirectory(rpc)
add_subdirectory(tools)
add_subdirectory(types)
add_subdirectory(version)
add_subdirectory(websockets)
