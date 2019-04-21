#include <chk/cassert.hpp>

#include <string>

namespace {
struct convert_to_bool {
	constexpr explicit operator bool() const noexcept {
		return true;
	}
};
struct convert_implicit_to_int {
	constexpr operator int() const noexcept {
		return 42;
	}
};

struct overloaded_op {
	constexpr bool operator!() const noexcept {
		return false;
	}
};

#if defined(__clang_major__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wunused-member-function"
#	pragma clang diagnostic ignored "-Wunneeded-member-function"
#endif
struct negation_takes_precedence_to_conversion {
	constexpr bool operator!() const noexcept {
		return false;
	}
	constexpr operator bool() const noexcept {
		return false;
	}
};
struct conversion_takes_precedence_to_call {
	constexpr explicit operator bool() const noexcept {
		return true;
	}
	constexpr bool operator()() const noexcept {
		return false;
	}
};

struct mutable_convert_to_bool {
	constexpr operator bool() noexcept {
		return true;
	}
};

struct uncopyable_empty_string {
	constexpr uncopyable_empty_string() = default;
	uncopyable_empty_string(uncopyable_empty_string&) = delete;
	constexpr uncopyable_empty_string(uncopyable_empty_string&&) = default;
	// minimal interface for working
	constexpr const char* c_str() const noexcept {
		return "";
	}
};
#if defined(__clang_major__)
#	pragma clang diagnostic pop
#endif

constexpr int cfun(int i) {
	chk::cassert(i > 0, "Hello");
	chk::cassert(i > 0, chk::detail::lambda_msg{"World"});
	chk::cassert(i > 0, uncopyable_empty_string{});
	chk::cassert(i > 0);
	chk::cassert(1);
	chk::cassert(convert_to_bool{});
	chk::cassert(overloaded_op{});
	chk::cassert(negation_takes_precedence_to_conversion{});
	chk::cassert(conversion_takes_precedence_to_call{});
	chk::cassert(convert_implicit_to_int{});
	return i;
}

int rfun(int i) {
	chk::cassert([&i]() { return i > 0; }, "lazy evaluated, constexpr since c++17");
	chk::cassert([&i]() { return i >= 0; });
	chk::cassert(i > 0, []() { return std::string("fail"); });
	chk::cassert(1, []() { return std::string("fail"); });
	chk::cassert(1, std::string("fail"));
	chk::cassert(1);
	chk::cassert(convert_to_bool{});
	chk::cassert(overloaded_op{});
	mutable_convert_to_bool v{};
	chk::cassert(v);
	return cfun(i);
}

static_assert(cfun(1) == 1, "no cassert triggered");

#if defined(_MSC_VER) || (!defined(__clang_major__) && __GNUC__ < 9)
constexpr bool test_fun(bool expr) {
	return noexcept(chk::detail::test_constexpr_helper(expr));
}
static_assert(test_fun(true), "inside static_assert it's always constexpr");
static_assert(test_fun(false), "inside static_assert it's always constexpr");
#endif

// ensure no accidental string copies

void fun2() {
	const char* c = chk::detail::call_or_convert_to_str_like("msg");
	(void) c;
	const std::string s = chk::detail::call_or_convert_to_str_like(std::string("msg"));  // FIXME: decltype, is_same, ...
	auto strlambda = []() { return std::string("msg"); };
	//chk::detail::zstr(strlambda);
	const std::string ss = chk::detail::call_or_convert_to_str_like(strlambda);
	auto cstrlambda = []() { return "msg"; };
	const char* cc = chk::detail::call_or_convert_to_str_like(cstrlambda);
	(void) cc;
	const auto& res = chk::detail::call_or_convert_to_str_like(uncopyable_empty_string{});
	(void) res;
}

template<class T> decltype(auto) fun3(const T& t) {
	return t;
}
}  // namespace

int main(int argv, char**) {
	const auto& f = fun3(uncopyable_empty_string{});
	(void) f;

	int arr[] = {1, 2, 3};
	const auto& f2 = fun3(arr);
	(void) f2;
	const int* f3 = fun3(arr);
	(void) f3;
	fun2();
	return rfun(argv) - argv;
}
