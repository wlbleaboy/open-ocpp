######################################################
#              OCPP 2.1 types library                #
######################################################

# Library target
file (GLOB OCPP21_TYPES_SOURCE_FILES "*.cpp")
add_library(types21 OBJECT
    ${OCPP21_TYPES_SOURCE_FILES}
)

# Exported includes
target_include_directories(types21 PUBLIC .)

# Dependencies
target_link_libraries(types21 PUBLIC types messages)
