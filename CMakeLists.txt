cmake_minimum_required (VERSION 2.6)
project(rlg)

set(CMAKE_CXX_STANDARD 11)

# Add my headers to the include path
include_directories(rlg PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(WIN32)
    include_directories("pdcurs36")

	# build pdcurses
	file(GLOB pdcurs_src "pdcurs36/wincon/*.c" "pdcurs36/pdcurses/*.c")

	add_library(pdcurs36 ${pdcurs_src})

	target_link_libraries(pdcurs36 ${EXTRA_LIBS})

	# Make sure rlg is the project that is open when visual studio starts
    if(${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT rlg)
    endif()
else()
	# Add some warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

	# Get ncurses
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
endif()

# NOTE: I add the headers here so they are visible in visual studio
file(GLOB rlg_src "src/*.c" "src/*.cpp" "include/*.h" "include/*.hpp")

add_executable(rlg ${rlg_src})

# Link libraries
if (WIN32)
    set(EXTRA_LIBS ${EXTRA_LIBS} pdcurs36)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CURSES_LIBRARIES})
endif()

target_link_libraries(rlg ${EXTRA_LIBS})
