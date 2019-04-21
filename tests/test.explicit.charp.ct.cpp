#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// char* should not convert implicitely to explicit type
	foo_explicit(foo_charp_data);
}
