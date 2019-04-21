#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// c_str should not be avaiable if not \0-terminated
	foo_explicit_nz_view_data.c_str();
}
