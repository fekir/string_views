#include <chk/unreachable.hpp>

namespace {
constexpr int fun(int i) {
	if (i > 0) {
		return i;
	}
	chk::unreachable();
}
}  // namespace

static_assert(fun(1) == 1, "no unreachable triggered");

int main(int argv, const char**) {
	return fun(argv) - argv;
}
