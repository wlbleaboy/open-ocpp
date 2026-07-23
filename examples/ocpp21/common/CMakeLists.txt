# Common utility classes for OCPP 2.1 examples
add_library(examples_common21 OBJECT
    DefaultChargePointEventsHandler21.cpp
)

target_include_directories(examples_common21 PUBLIC . config)

target_link_libraries(examples_common21 examples_common)
