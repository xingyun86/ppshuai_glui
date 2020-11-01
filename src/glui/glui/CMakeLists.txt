# CMakeList.txt : CMake project for glui, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(project_name glui)
project(${project_name})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/glew-2.2.0/include
	${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps
	${CMAKE_SOURCE_DIR}/glfw-3.3.2/include
	${CMAKE_BINARY_DIR}/glfw-3.3.2/src
	${CMAKE_SOURCE_DIR}/glm-0.9.9.8
)
link_directories(
	${CMAKE_BINARY_DIR}/glew-2.2.0/lib
	${CMAKE_BINARY_DIR}/glfw-3.3.2/src
	${CMAKE_BINARY_DIR}/glm-0.9.9.8
)
if (MSVC)
set(gl_ext_libs glew_s glfw3)
ELSE(MSVC)
set(gl_ext_libs glew_s glfw3 dl pthread)
ENDIF(MSVC)
if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(GLAD_GL "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/glad/gl.h"
            "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/glad_gl.c")
set(GLAD_VULKAN "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/glad/vulkan.h"
                "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/glad_vulkan.c")
set(GETOPT "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/getopt.h"
           "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/getopt.c")
set(TINYCTHREAD "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/tinycthread.h"
                "${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps/tinycthread.c")

if (${CMAKE_VERSION} VERSION_EQUAL "3.1.0" OR
    ${CMAKE_VERSION} VERSION_GREATER "3.1.0")
    set(CMAKE_C_STANDARD 99)
else()
    # Remove this fallback when removing support for CMake version less than 3.1
    add_compile_options("$<$<C_COMPILER_ID:AppleClang>:-std=c99>"
                        "$<$<C_COMPILER_ID:Clang>:-std=c99>"
                        "$<$<C_COMPILER_ID:GNU>:-std=c99>")
endif()

add_definitions(-DGLEW_STATIC)

# Add source to this project's executable.
add_executable (${project_name} "glui.cpp" "glui.h" ${GETOPT} ${GLAD_GL})

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

target_link_libraries(${project_name} ${gl_ext_libs})

add_executable (${project_name}-2 "glui2.cpp" "glui.h" ${GETOPT} ${GLAD_GL})

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

target_link_libraries(${project_name}-2 ${gl_ext_libs})

add_executable (${project_name}-3 "glui3.cpp" "glui.h" ${GETOPT} ${GLAD_GL})

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

target_link_libraries(${project_name}-3 ${gl_ext_libs})

add_executable (${project_name}-4 "glui4.cpp" "glui.h" ${GETOPT} ${GLAD_GL})

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

target_link_libraries(${project_name}-4 ${gl_ext_libs})
# TODO: Add tests and install targets if needed.
