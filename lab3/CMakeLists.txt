cmake_minimum_required(VERSION 3.10)

# Название проекта
project(lab3)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исходный файл
add_executable(lab3 main.cpp)

# Подключаем библиотеки OpenGL, GLU и GLUT
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(lab3 OpenGL::GL GLUT::GLUT GLU)
