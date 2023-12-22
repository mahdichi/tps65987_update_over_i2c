cmake_minimum_required(VERSION 3.0.0)
project(tps_flasher VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(tps_flasher 
    main.c
    i2c_cmd.c
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)



target_link_libraries(${PROJECT_NAME}  i2c )