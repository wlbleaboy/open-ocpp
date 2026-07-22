
# Common utility class for examples library
add_library(examples_common20 OBJECT
    DefaultChargePointEventsHandler.cpp
    DefaultBasicChargePointEventsHandler.cpp
    DefaultCentralSystemEventsHandler.cpp
    DefaultLocalControllerEventsHandler.cpp
    Ocpp20MeterValueProvider.cpp
)
target_include_directories(examples_common20 PUBLIC . config ${CMAKE_SOURCE_DIR}/examples/common/simulators)

# Dependencies
target_link_libraries(examples_common20 examples_common)
