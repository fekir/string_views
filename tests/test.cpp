#ifdef NDEBUG
#	undef NDEBUG
#endif

#include <string_views/string_views.hpp>

#include "test.foo.hpp"

#include <cassert>

#include <string>
#include <algorithm>
#include <memory>

using namespace string_views;

int main() {
	std::unique_ptr<char> pc;

	// assert that constant are in rodata or data.relro section
	{
		basic_string_views<char> from_literal(foo_charp_data);

		// test access and size
		assert(from_literal[0] == foo_charp_data[0]);
		assert(from_literal.front() == foo_charp_data[0]);
		assert(from_literal.back() == 'l');

		assert(from_literal[from_literal.size() - 1] == 'l');
		assert(from_literal.size() == 12);

		// constructors, from liteal already tested
		const char* data2 = from_literal.data();
		basic_string_views<char> from_const_char(data2);
		assert(from_const_char.size() == from_literal.size());
		assert(std::equal(from_const_char.begin(), from_const_char.end(), from_literal.begin()));
	}

	// constructors (FIXME: add conversion operators)
	// and implicit conversion
	{
		std::string str = foo_charp_data;
		basic_string_views<char> bsv(str);
		//	basic_string_views<char> bsv2 = str;

		foo_implicit(foo_charp_data);
		foo_implicit(str);
		foo_implicit(basic_string_views<char>(str));

		basic_string_views<char, conversion_policy::implicit_> implicit_constructor = foo_charp_data;
		implicit_constructor = str;
		(void) implicit_constructor;
		implicit_constructor = foo_charp_data;
		(void) implicit_constructor;
		implicit_constructor = basic_string_views<char, conversion_policy::explicit_>(foo_charp_data);
		(void) implicit_constructor;
		implicit_constructor = basic_string_views<char, conversion_policy::implicit_>(foo_charp_data);
		(void) implicit_constructor;
	}

	{
		//foo_explicit(basic_string_views<char, conversion_policy_e::implicit_>(foo_explicit_charp_data));  // do we want this to compile or not?
		foo_explicit(basic_string_views<char>(foo_charp_data));  // only this is allowed

		basic_string_views<char, conversion_policy::explicit_> explicit_constructor("");
		//explicit_constructor = str;
		//explicit_constructor = foo_explicit_charp_data;
		//explicit_constructor = basic_string_views<char, conversion_policy::implicit_>(foo_charp_data);
		explicit_constructor = basic_string_views<char, conversion_policy::explicit_>(foo_charp_data);
		(void) explicit_constructor;
	}

	{  // conversion from 0-terminated

		basic_string_views<char, conversion_policy::implicit_, format_policy::zero_terminated> zt("");
		basic_string_views<char, conversion_policy::implicit_, format_policy::not_zero_terminated> nzt = zt;
		(void) nzt;
		//		zt = nzt;  // should fail
	}

	{  // 0-term has c_str function
		foo_explicit_view_data.c_str();
		foo_implicit_view_data.c_str();
	}
	{
		assert(foo_explicit_view_data.c_str() == foo_explicit_view_data.data());
		assert(std::equal(
			 foo_explicit_view_data.begin(), foo_explicit_view_data.end(), std::begin(foo_arr_data), std::end(foo_arr_data) - 1));
	}

	{
		using t = basic_string_views<char>;
		static_assert(
			 std::is_same<decltype(std::declval<t>().begin()), decltype(std::declval<t>().end())>::value, "begin - end type mismatch");
		static_assert(std::is_same<decltype(std::declval<t>().begin()), decltype(std::declval<t>().cbegin())>::value,
			 "begin-cbegin type mismatch");
		static_assert(
			 std::is_same<decltype(std::declval<t>().begin()), decltype(std::declval<t>().end())>::value, "begin - end type mismatch");
		static_assert(std::is_same<decltype(std::declval<t>().cbegin()), decltype(std::declval<t>().cend())>::value,
			 "cbegin-cend type mismatch");
		static_assert(
			 std::is_same<decltype(std::declval<t>().data()), decltype(std::declval<t>().c_str())>::value, "data-c_str type mismatch");
	}

	{  // constexpr works
		constexpr auto data = foo_explicit_view_data.data();
		(void) data;
	}
	// test that those do not compile

	{  // policy
		struct test_policy1 {
			static constexpr void check(const char* begin, const char* end) {
				if (end == begin) {
					throw 42;
				}
			}
		};

		using t1 = basic_string_views<char, conversion_policy::explicit_, format_policy::zero_terminated, test_policy1>;

		try {
			auto data = t1{""};
			(void) data;
			assert(false);
		} catch (int) {
			auto data = t1{"12"};
			(void) data;
		}

		struct test_policy2 {
			static constexpr void check(const char* begin, const char* end) {
				if (end != begin) {
					throw 42.0;
				}
			}
		};

		using t2 = basic_string_views<char, conversion_policy::explicit_, format_policy::zero_terminated, test_policy2>;
		try {
			auto data = t2{t1{""}};  // explicit conversion
			(void) data;
			assert(false);
		} catch (int) {
		}
		try {
			auto data0 = t1{"!"};
			auto data = t2{data0};  // explicit conversion
			(void) data;
			assert(false);
		} catch (double) {
		}

		using t3 = basic_string_views<char, conversion_policy::implicit_, format_policy::zero_terminated, test_policy2>;
		try {
			t3 data = t1{""};  // implicit works even with different policies...
			(void) data;
			assert(false);
		} catch (int) {
		}
	}

	{  // signed and unsigned are both valid indexes
		foo_explicit_view_data[1];
		foo_explicit_view_data[1u];
	}

	{  // constructor with default policy is noexcept
		static_assert(noexcept(ex_z_string_view("")), "");
	}

	{  // substr
		(void) foo_explicit_view_data.substr(1);
		(void) foo_explicit_nz_view_data.substr(1, 1);
	}
}
