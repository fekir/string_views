#include <string_views/string_views.hpp>

#include "test.foo.hpp"

int main() {
	// nz should not convert to z implicitely
	im_z_string_view data = foo_explicit_nz_view_data;
}
