cmake_minimum_required(VERSION 2.9)

project(SPAD)

file(GLOB SOURCES *.cpp *.cc)
file(GLOB HEADER *.hpp *.hh)
# Include header files

include_directories(source)

# Create shared library
add_library(${PROJECT_NAME} SHARED  ${SOURCES} ${HEADER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set (CMAKE_CXX_STANDARD 17)

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

# Install library headers
install(FILES ${HEADER} DESTINATION ${PROJECT_NAME})
