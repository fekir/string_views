#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// implicit should not convert implicitely to explicit type
	foo_explicit(foo_implicit_view_data);  //--should not compile
}
