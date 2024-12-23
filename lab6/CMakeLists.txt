cmake_minimum_required(VERSION 3.10)

# Название проекта
project(lab6)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем директорию заголовочных файлов
include_directories(${CMAKE_SOURCE_DIR}/include)

# Указываем все исходные файлы проекта
set(SRC_FILES
    src/main.cpp
    src/camera.cpp
    src/objects.cpp
    src/lighting.cpp
    src/hud.cpp
    src/render.cpp
)

# Добавляем исполняемый файл
add_executable(lab6 ${SRC_FILES})

# Подключаем библиотеки OpenGL, GLU и GLUT
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(lab6 OpenGL::GL GLUT::GLUT GLU)
