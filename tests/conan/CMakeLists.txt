cmake_minimum_required(VERSION 3.11)
project(test-chk-conan VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	find_program(WINE_EXECUTABLE NAMES wine REQUIRED)
	set(TARGET_SYSTEM_EMULATOR ${WINE_EXECUTABLE})
	if(MINGW)
		# for testing in wine...
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
	endif()
endif()

include(CTest REQUIRED)
if(EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
	include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()


find_package(chk REQUIRED)
 
add_executable(test.unreachable.success test.unreachable.success.cpp)
target_link_libraries(test.unreachable.success chk::chk)
add_test(NAME unreachable.success COMMAND ${TARGET_SYSTEM_EMULATOR} $<TARGET_FILE:test.unreachable.success>)

add_executable(test.cassert.success test.cassert.success.cpp)
target_link_libraries(test.cassert.success chk::chk)
add_test(NAME cassert.success COMMAND ${TARGET_SYSTEM_EMULATOR} $<TARGET_FILE:test.cassert.success>)

set_tests_properties(
		unreachable.success cassert.success
	PROPERTIES
		ENVIRONMENT
			WINEPREFIX=$ENV{HOME}/.wine # FIXME: caller should optionally provide test env, check if WINEPREFIX already set
			# also set WINEPATH to ensure shared dlls from msvc are find in "naked" wine env -> alternative to static linking mingw
				# (https://stackoverflow.com/questions/7249791/wine-path-through-command-line-and-not-persistent)
			# DLL_PATH_ENV ?
				# https://github.com/clibs/cmocka/blob/master/example/CMakeLists.txt
)


########## FAILING TESTS ##########

add_executable(test.unreachable.ct test.unreachable.ct.cpp)
target_link_libraries(test.unreachable.ct chk::chk)
add_test(NAME unreachable.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.unreachable.ct
)

add_executable(test.cterminate.ct test.cterminate.ct.cpp)
target_link_libraries(test.cterminate.ct chk::chk)
add_test(NAME cterminate.ct
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.cterminate.ct
)

add_executable(test.cassert.ct test.cassert.ct.cpp)
target_link_libraries(test.cassert.ct chk::chk)
add_test(NAME cassert.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.cassert.ct
)

set_target_properties(
        test.unreachable.ct test.cassert.ct test.cterminate.ct
	PROPERTIES
		EXCLUDE_FROM_ALL TRUE
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
)

set_tests_properties(
		unreachable.ct cassert.ct cterminate.ct
	PROPERTIES
		WILL_FAIL TRUE
)
