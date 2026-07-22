set(project_version "V01.00.000")
# set(project_name ocppclient_${project_version})
set(project_name local_test)
project(${project_name})

cmake_minimum_required(VERSION 3.9)

message(${CMAKE_CURRENT_SOURCE_DIR})

get_filename_component(SECOND_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

message(${SECOND_FOLDER_ABSOLUTE})

get_filename_component(THIRD_FOLDER_ABSOLUTE ${SECOND_FOLDER_ABSOLUTE} DIRECTORY)
message(${THIRD_FOLDER_ABSOLUTE})

get_filename_component(FOURTH_FOLDER_ABSOLUTE ${THIRD_FOLDER_ABSOLUTE} DIRECTORY)
message(${FOURTH_FOLDER_ABSOLUTE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型为调试模式
set(CMAKE_BUILD_TYPE Debug)

# 添加编译选项，启用调试信息
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 添加链接选项，包含调试信息
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

# Warnings
set(WARNING_FLAGS "-Wall -Werror")

# C/C++ flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set compiler flags
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG_INIT} ${WARNING_FLAGS} -O0 -g3 -ggdb3")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG_INIT} ${WARNING_FLAGS} -O0 -g3 -ggdb3")
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE_INIT} ${WARNING_FLAGS} -O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} ${WARNING_FLAGS} -O2 -DNDEBUG")

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)
set(CONFIG)
set(SIMU
    ${PROJECT_SOURCE_DIR}/common/simulators/MainMeterSimulator.cpp
    ${PROJECT_SOURCE_DIR}/common/simulators/MeterSimulator.cpp
)
set(COMMON
    ${PROJECT_SOURCE_DIR}/common/DefaultChargePointEventsHandler.cpp
    ${PROJECT_SOURCE_DIR}/common/Ocpp20MeterValueProvider.cpp
    ${PROJECT_SOURCE_DIR}/chargerManager/ChargerManager20.cpp
    ${PROJECT_SOURCE_DIR}/chargerManager/SetpointManager.cpp
)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${CONFIG} ${SIMU} ${COMMON})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${FOURTH_FOLDER_ABSOLUTE}/src/ocpp20/chargepoint/interface
    ${FOURTH_FOLDER_ABSOLUTE}/src/ocpp20/chargepoint/smartcharging
    ${FOURTH_FOLDER_ABSOLUTE}/output/include/openocpp
    ${PROJECT_SOURCE_DIR}/common/config
    ${PROJECT_SOURCE_DIR}/common/simulators
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/chargerManager
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)
link_directories(
    ${FOURTH_FOLDER_ABSOLUTE}/output/examples
)
target_link_libraries(${PROJECT_NAME} ${FOURTH_FOLDER_ABSOLUTE}/output/examples/libopen-ocpp.so -lzlog -lpthread -lssl -lcrypto)
