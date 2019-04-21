#include <chk/cassert.hpp>

namespace {
constexpr int fun(int i) {
	chk::cassert(i > 0, "Hello");
	chk::cassert(i > 0);
	return i;
}
}  // namespace

static_assert(fun(-1) == -1, "no cassert triggered");

int main() {
}
