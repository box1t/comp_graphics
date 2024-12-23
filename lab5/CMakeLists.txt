cmake_minimum_required(VERSION 3.10)

# Название проекта
project(lab5)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Включение OpenMP (если доступно)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Укажите исходные файлы проекта
set(SOURCES main.cpp)

# Добавляем исполняемый файл
add_executable(lab5 ${SOURCES})

# Убедитесь, что компилятор оптимизирован для производительности
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()

# Убедитесь, что файлы STB находятся в том же каталоге, что и `main.cpp`
target_include_directories(lab5 PRIVATE ${CMAKE_SOURCE_DIR})
