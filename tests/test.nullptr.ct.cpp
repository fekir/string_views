#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// nullptr should not convert
	foo_implicit(nullptr);
}
