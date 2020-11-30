#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// bool is not a valid index type
	(void) foo_explicit_view_data[false];
}
