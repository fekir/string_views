#include <chk/cterminate.hpp>

namespace {
constexpr int fun(int i) {
	chk::cterminate();
	return i;
}
}  // namespace

static_assert(fun(-1) == -1, "no unreachable triggered");

int main() {
}
