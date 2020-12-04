#include <string_views/string_views.hpp>

#include "test.foo.hpp"

struct empty_content_policy {
	static constexpr void check(const char*, const char*) noexcept {
	}
};

int main() {
	using namespace string_views;
	using str = basic_string_views<char, conversion_policy::explicit_, format_policy::not_zero_terminated, empty_content_policy>;
	// non \0-terminated should not have substr taking a lenght
	str("").substr(1);
}
