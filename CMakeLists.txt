cmake_minimum_required(VERSION 3.13)

project(uMarioU CXX)

enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The engine is a C++/SDL2 codebase written for MSVC on Windows; it relies on
# permissive conversions and does not use exceptions.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fno-exceptions -fno-rtti -Wno-write-strings")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------

file(GLOB UMARIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/uNext/*.cpp")
file(GLOB PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/*.cpp")

add_executable(uMarioU
	${UMARIO_SOURCES}
	${PLATFORM_SOURCES}
)

target_include_directories(uMarioU PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/uNext
	${CMAKE_CURRENT_SOURCE_DIR}/src/platform
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
)

target_link_directories(uMarioU PRIVATE
	${SDL2_LIBRARY_DIRS}
	${SDL2_IMAGE_LIBRARY_DIRS}
	${SDL2_MIXER_LIBRARY_DIRS}
)

target_link_libraries(uMarioU PRIVATE
	${SDL2_MIXER_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${SDL2_LIBRARIES}
)

target_link_options(uMarioU PRIVATE
	-Wl,--gc-sections
)

# -----------------------------------------------------------------------------
# .rpx / .wuhb packaging
# -----------------------------------------------------------------------------

wut_create_rpx(uMarioU)

set(CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/content")

wut_create_wuhb(uMarioU
	CONTENT   "${CONTENT_DIR}"
	NAME      "uMario U"
	SHORTNAME "uMario U"
	AUTHOR    "PortBREW"
	ICON      "${CMAKE_CURRENT_SOURCE_DIR}/meta/icon.png"
	TVSPLASH  "${CMAKE_CURRENT_SOURCE_DIR}/meta/tv-splash.png"
	DRCSPLASH "${CMAKE_CURRENT_SOURCE_DIR}/meta/drc-splash.png"
)
