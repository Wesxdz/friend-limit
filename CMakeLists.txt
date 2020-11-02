cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GAME_NAME friendlimit)
set(SOURCE_PATH "src") # Relative to PROJECT_SOURCE_DIR
set(GAME_LAMBDAS "${GAME_NAME}_lambdas")

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# You can check for this in C++ with '#ifdef HOT_RELOAD'
set(HOT_RELOAD True)

project(${GAME_NAME})

file(GLOB_RECURSE SOURCES "${SOURCE_PATH}/*.cpp")

if(HOT_RELOAD)
# Will fail to build (NO SOURCES) if there are no SunLambdas
	file(GLOB_RECURSE SUNLAMBDAS_SOURCES "${SOURCE_PATH}/SunLambdas/*.cpp")
	add_library(${GAME_LAMBDAS} "${SUNLAMBDAS_SOURCES}")

	add_compile_definitions("${GAME_NAME}" HOT_RELOAD)
	add_compile_definitions("${GAME_LAMBDAS}" HOT_RELOAD)
endif()

add_executable(${GAME_NAME} ${SOURCES})

include_directories("${SOURCE_PATH}/")

set(PROJECT_EXT_DIR "${PROJECT_SOURCE_DIR}/submodule")

if ( CMAKE_COMPILER_IS_GNUCC  )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused")
elseif ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

list(APPEND LINK_LIBRARIES sfml-graphics sfml-window sfml-system sfml-audio)

set(CMAKE_PREFIX_PATH "${PROJECT_EXT_DIR}/SFML/cmake")

if (UNIX) #🐧❤️
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -fconcepts -Wunused-variable")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	list(APPEND LINK_LIBRARIES ${X11_LIBRARIES})
endif(UNIX)

target_link_libraries(${GAME_NAME} ${LINK_LIBRARIES})

if(HOT_RELOAD)
	target_link_libraries(${GAME_NAME} ${CMAKE_DL_LIBS})
	target_link_libraries(${GAME_LAMBDAS} ${LINK_LIBRARIES})

	add_compile_definitions(${GAME_NAME} HOT_RELOAD_CMAKE="${CMAKE_COMMAND}")
	add_compile_definitions(${GAME_NAME} HOT_RELOAD_TARGET="${GAME_LAMBDAS}")
	add_compile_definitions(${GAME_NAME} HOT_RELOAD_BUILD_PATH="${CMAKE_BINARY_DIR}")
	add_compile_definitions(${GAME_NAME} HOT_RELOAD_LIB="$<TARGET_FILE:${GAME_LAMBDAS}>")
endif()

add_custom_command(TARGET ${GAME_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${GAME_NAME}>/resources
)
