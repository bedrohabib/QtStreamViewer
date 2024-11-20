# Minimum CMake version required
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Project name and version
project(stream VERSION 1.0 LANGUAGES CXX)

# Find the Qt5 package with necessary modules
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets  REQUIRED)
find_package(Threads REQUIRED)

set(HEADERS
ReceptionWorker.h
DecodingWorker.h
RenderingWorker.h
)

qt5_wrap_cpp(MOC_SOURCES ${HEADERS}) # Ensures headers with Q_OBJECT are processed

# Specify the source files
set(SOURCES main.cpp)

# Find FFmpeg libraries using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
include_directories(${FFMPEG_INCLUDE_DIRS})

# Add the executable target
add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES})

# Link the necessary Qt libraries
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets)

# Link FFmpeg libraries
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})