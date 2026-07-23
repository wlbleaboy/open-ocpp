######################################################
#            OCPP 2.1 messages library               #
######################################################

# Library target
file (GLOB OCPP21_MESSAGES_SOURCE_FILES "*.cpp")
add_library(messages21 OBJECT
    ${OCPP21_MESSAGES_SOURCE_FILES}
)

# Exported includes
target_include_directories(messages21 PUBLIC .)

# Dependencies
target_link_libraries(messages21 PUBLIC types21 messages)
