project(test-string_views VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	find_program(WINE_EXECUTABLE NAMES wine REQUIRED)
	set(TARGET_SYSTEM_EMULATOR ${WINE_EXECUTABLE})
	if(MINGW)
		# for testing in wine...
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
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
endif()

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
function(add_failing_ct_test id error_msg)
    set(id "${id}.ct")
    add_executable("test.${id}" EXCLUDE_FROM_ALL "test.${id}.cpp")
    target_link_libraries("test.${id}" string_views::string_views)
    add_test(NAME "${id}"
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target "test.${id}"
    )
    set_tests_properties("${id}"
	PROPERTIES
	    PASS_REGULAR_EXPRESSION "${error_msg}"
    )
endfunction()


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
set(TEST_NO_VALID_INDEX_TYPE ".*valid index.*")

add_failing_ct_test(explicit.charp "${TEST_CONVERSION_ERROR}")
add_failing_ct_test(explicit.string "${TEST_CONVERSION_ERROR}")
add_failing_ct_test(explicit.implicit_view "${TEST_CONVERSION_ERROR}")

add_failing_ct_test(nullptr "${TEST_DELETED_FUNCTION_ERROR}")
add_failing_ct_test(format.implicit "${TEST_FORMAT_POLICY_ERROR}")
add_failing_ct_test(format.explicit "${TEST_FORMAT_POLICY_ERROR}")


add_failing_ct_test(nz.c_str "${TEST_NO_FUNCTION_ERROR}")
add_failing_ct_test(crippled.substr "${TEST_NO_FUNCTION_ERROR}")

add_failing_ct_test(bool.index "${TEST_NO_VALID_INDEX_TYPE}")
