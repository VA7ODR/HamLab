get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(${MODULE_NAME} STATIC
    ${MODULE_NAME}.cpp ${MODULE_NAME}.hpp
)

target_include_directories(${MODULE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${MODULE_NAME} PRIVATE
    easy_app_base
)
