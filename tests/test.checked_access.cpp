#ifdef NDEBUG
#	undef NDEBUG
#endif

#include "test.foo.hpp"

using namespace string_views;

int main(int argc, char**) {
	using sv = string_views::basic_string_views<char, string_views::conversion_policy::explicit_,
		 string_views::format_policy::zero_terminated, default_content_policy<char>, string_views::debug_policy::checked>;
	auto data = sv(foo_implicit_view_data);
	return static_cast<int>(data[data.size()]) == argc;  // should abort
}
