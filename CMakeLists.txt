cmake_minimum_required(VERSION 3.23)

if(NOT DEFINED ENV{VCPKG_ROOT})
	message(FATAL_ERROR "VCPKG_ROOT environment variable is not set")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(wipeout)
enable_language(C CXX)

if(APPLE)
	enable_language(OBJC)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)

add_executable(wipeout)
add_subdirectory(external)

if(MSVC)
	target_compile_options(wipeout PRIVATE /W4)
else()
	target_compile_options(wipeout PRIVATE -Wall -Wextra)
endif()

target_link_libraries(wipeout PRIVATE SDL3::SDL3)

if(TARGET OpenGL::EGL)
	target_link_libraries(wipeout PUBLIC OpenGL::EGL)
elseif(TARGET OpenGL::OpenGL)
	target_link_libraries(wipeout PUBLIC OpenGL::OpenGL)
else()
	target_link_libraries(wipeout PUBLIC OpenGL::GL)
endif()

target_link_libraries(wipeout PRIVATE GLEW::GLEW)

if(WIN32)
	target_compile_definitions(wipeout 
		PRIVATE
			"NOMINMAX"
			"_USE_MATH_DEFINES"
			"_CRT_SECURE_NO_WARNINGS"
	)
elseif(APPLE)
	target_compile_definitions(wipeout 
		PRIVATE
			"_THREAD_SAFE"
			"GL_SILENCE_DEPRECATION"
	)

	target_link_libraries(wipeout PUBLIC "-framework Foundation")
endif()

target_link_libraries(wipeout PRIVATE external)

target_sources(wipeout
	PRIVATE
		FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/include FILES
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/camera.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/droid.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/game.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/hud.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/image.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/ingame_menus.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/intro.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/main_menu.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/menu.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/object.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/particle.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/race.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/scene.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/sfx.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/ship.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/ship_ai.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/ship_player.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/title.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/track.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/ui.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/wipeout/weapon.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/input.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/mem.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/platform.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/render.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/system.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/types.h
			${CMAKE_CURRENT_SOURCE_DIR}/source/include/utils.h
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/camera.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/droid.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/game.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/hud.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/image.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/ingame_menus.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/intro.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/main_menu.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/menu.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/object.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/particle.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/race.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/scene.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/sfx.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/ship.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/ship_ai.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/ship_player.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/title.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/track.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/ui.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/wipeout/weapon.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/input.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/mem.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/platform.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/render.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/system.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/types.c
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/utils.c
)

if(WIN32)
	target_sources(wipeout
		PRIVATE
			packaging/windows/wipeout.exe.manifest
			packaging/windows/wipeout.rc
	)
endif()
