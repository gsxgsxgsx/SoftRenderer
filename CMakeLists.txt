cmake_minimum_required(VERSION 3.0.0)
project(TinyRenderer VERSION 0.1.0)


include(CTest)
enable_testing()
# 使用 C++ 11 标准
set(CMAKE_CXX_STANDARD 11)

#添加头文件
set(GLAD_H ${PROJECT_SOURCE_DIR}/include)

set(GLAD_H /usr/local/include)
set(KHR_H /usr/local/include)
set(GLFW_H /usr/local/include)
set(STB_IMAGE_H /usr/local/include)
include_directories(${GLAD_H} ${GLFW_H} ${KHR_H} ${STB_IMAGE_H})

# 添加目标链接
set(GLFW_LINK /usr/local/lib/libglfw.3.dylib)
link_libraries(${GLFW_LINK})


# 执行编译命令
set(SOURCES  data_model.cpp geometry.cpp math.cpp our_gl.cpp data_input.cpp main.cpp)

add_executable(TinyRenderer ${SOURCES})

# 链接系统的 OpenGL 框架
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
