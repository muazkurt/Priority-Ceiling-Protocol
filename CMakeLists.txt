cmake_minimum_required(VERSION 3.15)
project(Priority_Ceiling_Protocol)

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
            FATAL: In-source builds are not allowed.
                You should create a separate directory for build files.
            ")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package (Threads)

if( DEBUG ) 
    message("Entering Debug all mode.")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
    add_compile_definitions(DEBUG)
endif()

if(DEBUG_2)
    message("Entering Debug only in final mode.")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_2")
    add_compile_definitions(DEBUG_2)
endif()

add_executable(Priority_Ceiling_Protocol main.cpp gtu_mutex/mutex.cpp gtu_mutex/mutex.h)


target_link_libraries(Priority_Ceiling_Protocol ${CMAKE_THREAD_LIBS_INIT})
