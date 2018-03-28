cmake_minimum_required (VERSION 2.6)
project(rlg)

set(CMAKE_CXX_STANDARD 11)

include_directories("include")

if(WIN32)
    include_directories("pdcurs36")

    if(${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT rlg)
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
endif()

file(GLOB rlg_src "src/*.c" "src/*.cpp" "include/*.h" "include/*.hpp")

add_executable(rlg ${rlg_src})

if (WIN32)
    set(EXTRA_LIBS ${EXTRA_LIBS} pdcurs36/pdcurses)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CURSES_LIBRARIES})
endif()

target_link_libraries(rlg ${EXTRA_LIBS})
