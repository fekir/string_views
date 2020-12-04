#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// non \0-terminated should not have substr taking a lenght
	foo_explicit_view_data.substr(1, 1);  //--should not compile
}
