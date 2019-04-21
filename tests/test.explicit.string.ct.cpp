#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// std::string should not convert implicitely to explicit type
	foo_explicit(foo_string_data);  //--should not compile
}
