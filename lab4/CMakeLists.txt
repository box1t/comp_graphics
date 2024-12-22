cmake_minimum_required(VERSION 3.10)

# Название проекта
project(lab4)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Пути к GLEW
set(GLEW_INCLUDE_DIRS /usr/include/GL)
set(GLEW_LIBRARIES /usr/lib/x86_64-linux-gnu/libGLEW.so)

# Пути к GLFW
set(GLFW_INCLUDE_DIRS /usr/include/GLFW)
set(GLFW_LIBRARIES /usr/lib/x86_64-linux-gnu/libglfw.so)

# Пути к GLM (добавляем вручную, если не работает find_package)
set(GLM_INCLUDE_DIRS /usr/include)

# Добавляем исходный файл
add_executable(lab4 main.cpp)

# Указываем пути к библиотекам
find_package(OpenGL REQUIRED)

# Подключаем библиотеки
target_include_directories(lab4 PRIVATE 
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

target_link_libraries(lab4 PRIVATE 
    OpenGL::GL 
    ${GLEW_LIBRARIES} 
    ${GLFW_LIBRARIES}
)
