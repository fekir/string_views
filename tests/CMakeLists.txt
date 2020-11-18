cmake_minimum_required(VERSION 3.11)
project(test-string_views VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	find_program(WINE_EXECUTABLE NAMES wine REQUIRED)
	set(TARGET_SYSTEM_EMULATOR ${WINE_EXECUTABLE})
	if(MINGW)
		# for testing in wine...
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
	endif()
endif()

#eases testing in wine...

#since cmake 3.15: https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELWITHDEBINFO
        )
foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
endforeach()


add_executable(mytest test.cpp test.foo.hpp)
target_link_libraries(mytest string_views::string_views)
add_test(NAME mytest COMMAND ${TARGET_SYSTEM_EMULATOR} $<TARGET_FILE:mytest>)


#set_tests_properties(
#	mytest
#	PROPERTIES
#		ENVIRONMENT
#			WINEPREFIX=$ENV{HOME}/.wine # FIXME: caller should optionally provide test env, check if WINEPREFIX already set
#			# also set WINEPATH to ensure shared dlls from msvc are find in "naked" wine env -> alternative to static linking mingw
#				# (https://stackoverflow.com/questions/7249791/wine-path-through-command-line-and-not-persistent)
#			# DLL_PATH_ENV ?
#				# https://github.com/clibs/cmocka/blob/master/example/CMakeLists.txt
#)
########## FAILING TESTS ##########
add_executable(test.explicit.charp.ct EXCLUDE_FROM_ALL test.explicit.charp.ct.cpp)
target_link_libraries(test.explicit.charp.ct string_views::string_views)
add_test(NAME explicit.charp.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.explicit.charp.ct
)
add_executable(test.nullptr.ct EXCLUDE_FROM_ALL test.nullptr.ct.cpp)
target_link_libraries(test.nullptr.ct string_views::string_views)
add_test(NAME nullptr.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.nullptr.ct
)
add_executable(test.explicit.string.ct EXCLUDE_FROM_ALL test.explicit.string.ct.cpp)
target_link_libraries(test.explicit.string.ct string_views::string_views)
add_test(NAME explicit.string.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.explicit.string.ct
)
add_executable(test.explicit.implicit_view.ct EXCLUDE_FROM_ALL test.explicit.implicit_view.ct.cpp)
target_link_libraries(test.explicit.implicit_view.ct string_views::string_views)
add_test(NAME explicit.implicit_view.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.explicit.implicit_view.ct
)
add_executable(test.format.implicit.ct EXCLUDE_FROM_ALL test.format.implicit.ct.cpp)
target_link_libraries(test.format.implicit.ct string_views::string_views)
add_test(NAME format.implicit.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.format.implicit.ct
)
add_executable(test.format.explicit.ct EXCLUDE_FROM_ALL test.format.explicit.ct.cpp)
target_link_libraries(test.format.explicit.ct string_views::string_views)
add_test(NAME format.explicit.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.format.explicit.ct
)
add_executable(test.nz.c_str.ct EXCLUDE_FROM_ALL test.nz.c_str.ct.cpp)
target_link_libraries(test.nz.c_str.ct string_views::string_views)
add_test(NAME nz.c_str.ct
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test.nz.c_str.ct
)

add_executable(test.checked_access test.checked_access.cpp)
target_link_libraries(test.checked_access string_views::string_views)
add_test(NAME checked_access COMMAND sh -c "${TARGET_SYSTEM_EMULATOR} $<TARGET_FILE:test.checked_access>")
set_tests_properties(
	checked_access
	PROPERTIES
		PASS_REGULAR_EXPRESSION ".*access.*"
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(TEST_CONVERSION_ERROR ".* no known conversion from .*")
	set(TEST_NO_FUNCTION_ERROR ".* no matching .* function for call to .*") # no matching member function for call to
        set(TEST_DELETED_FUNCTION_ERROR ".*has been explicitly marked deleted.*")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(TEST_CONVERSION_ERROR ".* error: could not convert .*")
	set(TEST_NO_FUNCTION_ERROR ".* no matching function for call to .*")
        set(TEST_DELETED_FUNCTION_ERROR ".*use of deleted function.*")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(TEST_CONVERSION_ERROR ".* error C2664: .* cannot convert .*")
	set(TEST_NO_FUNCTION_ERROR ".* no matching overloaded function found.*")
else()
	message(FAIL "No known error message for ${CMAKE_CXX_COMPILER_ID}")
endif()

set(TEST_FORMAT_POLICY_ERROR ".*conversion to zero_terminated not allowed.*")

set_tests_properties(
	explicit.charp.ct explicit.string.ct explicit.implicit_view.ct
	PROPERTIES
		#WILL_FAIL TRUE
		PASS_REGULAR_EXPRESSION "${TEST_CONVERSION_ERROR}"
)
set_tests_properties(
	nullptr.ct
	PROPERTIES
		#WILL_FAIL TRUE
		PASS_REGULAR_EXPRESSION "${TEST_DELETED_FUNCTION_ERROR}"
)
set_tests_properties(
	format.implicit.ct format.explicit.ct
	PROPERTIES
		#WILL_FAIL TRUE
		PASS_REGULAR_EXPRESSION "${TEST_FORMAT_POLICY_ERROR}"
)
set_tests_properties(
	nz.c_str.ct
	PROPERTIES
		#WILL_FAIL TRUE
		PASS_REGULAR_EXPRESSION "${TEST_NO_FUNCTION_ERROR}"
)
