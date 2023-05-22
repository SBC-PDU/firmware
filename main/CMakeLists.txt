FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include/
)

target_compile_definitions(${COMPONENT_TARGET} PRIVATE "-DREVISION=3")

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../webapp")
if(EXISTS ${WEB_SRC_DIR}/dist)
    spiffs_create_partition_image(spiffs ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
endif()
