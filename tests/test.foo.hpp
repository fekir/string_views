#include <string_views/string_views.hpp>

#include <string>

constexpr const char foo_arr_data[] = "from_literal";
constexpr char const* const foo_charp_data = foo_arr_data;
#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wexit-time-destructors"
#	pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
const std::string foo_string_data = foo_arr_data;
#ifdef __clang__
#	pragma clang diagnostic pop
#endif

// FIXME: need to define assign oeprators too
using ex_z_string_view =
     string_views::basic_string_views<char, string_views::conversion_policy::explicit_, string_views::format_policy::zero_terminated>;
using im_z_string_view =
     string_views::basic_string_views<char, string_views::conversion_policy::implicit_, string_views::format_policy::zero_terminated>;
using ex_nz_string_view = string_views::basic_string_views<char, string_views::conversion_policy::explicit_,
     string_views::format_policy::not_zero_terminated>;
using im_nz_string_view = string_views::basic_string_views<char, string_views::conversion_policy::implicit_,
     string_views::format_policy::not_zero_terminated>;

constexpr ex_z_string_view foo_explicit_view_data(foo_charp_data);
constexpr const im_z_string_view foo_implicit_view_data(foo_charp_data);
constexpr const ex_nz_string_view foo_explicit_nz_view_data(foo_charp_data);
constexpr const im_nz_string_view foo_implicit_nz_view_data(foo_charp_data);

inline void foo_explicit(ex_z_string_view explicit_constructor) {
	(void) explicit_constructor;
}

inline void foo_implicit(im_z_string_view implicit_constructor) {
	(void) implicit_constructor;
}
