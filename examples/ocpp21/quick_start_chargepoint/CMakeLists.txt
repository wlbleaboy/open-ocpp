######################################################
#     Quick start OCPP 2.1 charge point example     #
######################################################

add_executable(quick_start_chargepoint21
    main.cpp
)

target_link_directories(quick_start_chargepoint21 PRIVATE ${BIN_DIR})

target_link_libraries(quick_start_chargepoint21
    examples_common21
)

ADD_CUSTOM_COMMAND(TARGET quick_start_chargepoint21
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/config/quick_start_chargepoint21.ini ${BIN_DIR}/
)
