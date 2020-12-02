#pragma once

#include <iterator>

#include <cstddef>      // ptrdiff
#include <type_traits>  // std::enable_if, std::integral_constant
#include <cassert>

namespace string_views {

// everything explicit, minimal interface
// plus empty, as convenience
template<typename character>  //
struct buff_view {
	constexpr buff_view(const character* data_, std::size_t size_) noexcept : m_data{data_}, m_size{size_} {
	}
	constexpr const character* data() const noexcept {
		return m_data;
	}
	constexpr std::size_t size() const noexcept {
		return m_size;
	}
	constexpr bool empty() const noexcept {
		return this->size() == 0;
	}

private:
	const character* m_data;
	std::size_t m_size;
};

template<class character> struct default_content_policy {
	static constexpr void check(const character* begin, const character* end) noexcept {
		(void) begin;
		(void) end;
	}
};

template<typename character, class content_policy>  //
class checked_buff_view : private buff_view<character> {
	using base = buff_view<character>;

public:
	constexpr static bool construct_noexcept = noexcept(content_policy::check(nullptr, nullptr));
	constexpr checked_buff_view(const character* begin_, std::size_t size_) noexcept(construct_noexcept) : base(begin_, size_) {
		content_policy::check(this->data(), this->data() + this->size());
	}
	using base::data;
	using base::size;
	using base::empty;
};

namespace detail {
template<typename character> unsigned long constexpr len(const character* str) noexcept {
	return *str ? 1u + len(++str) : 0u;
}
}  // namespace detail
// should this apply to both const char* and std::string? what about operator const char*?
enum class conversion_policy {
	implicit_,
	explicit_,
};  // FIXME: find better name as explicit reserved

template<class character = char,                                                        //
     conversion_policy explicit_constructor_from_charp = conversion_policy::explicit_,  //
     class content_policy = default_content_policy<character>                           //
     >
class create_checked_buff_view_from_str : private checked_buff_view<character, content_policy> {
	using base = checked_buff_view<character, content_policy>;

public:
	using base::construct_noexcept;
	constexpr create_checked_buff_view_from_str(const character* begin_, const character* end_) noexcept(construct_noexcept) :
	            base(begin_, static_cast<std::size_t>(end_ - begin_)) {
	}
	constexpr create_checked_buff_view_from_str(const character* begin_, std::size_t size_) noexcept(construct_noexcept) :
	            base(begin_, size_) {
	}
	constexpr create_checked_buff_view_from_str(std::nullptr_t) = delete;

	// convert from character*
	template<typename _Dummy = void,
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::explicit_) && std::is_void<_Dummy>::value,
	          character>::type = false>
	constexpr explicit create_checked_buff_view_from_str(const character* data_) noexcept(construct_noexcept) :
	            base(data_, detail::len(data_)) {
	}

	template<typename _Dummy = void,
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_) && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr create_checked_buff_view_from_str(const character* data_) noexcept(construct_noexcept) : base(data_, detail::len(data_)) {
	}

	// convert from std::string
	template<class string,
	     typename std::enable_if<explicit_constructor_from_charp == conversion_policy::explicit_ && !std::is_void<string>::value,
	          character>::type = true>
	constexpr explicit create_checked_buff_view_from_str(const string& s) noexcept(construct_noexcept) : base(s.c_str(), s.size()) {
	}
	template<class string,
	     typename std::enable_if<explicit_constructor_from_charp == conversion_policy::implicit_ && !std::is_void<string>::value,
	          character>::type = true>
	constexpr create_checked_buff_view_from_str(const string& s) noexcept(construct_noexcept) : base(s.c_str(), s.size()) {
	}

	// convert from std::string_view
	// ???

	using base::data;
	using base::size;
	using base::empty;
};

enum class format_policy {
	zero_terminated,
	not_zero_terminated,
};

enum debug_policy {
	unchecked,
	checked,
#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL >= 1 && defined(_MSC_VER)
	global = checked,
#else
	global = unchecked,
#endif
};  // FIXME: implementation is incomplete

// constexpr strlen
namespace detail {
template<enum debug_policy p> using iterator_type = std::integral_constant<debug_policy, p>;
using unchecked_debug = iterator_type<debug_policy::unchecked>;
using checked_debug = iterator_type<debug_policy::checked>;

// FIXME: add conversion tests and that it helps to catch bugs
// could save checked iter instead of pointer-type?
// as downside c_str/data gets more complicated...
#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL >= 1 && defined(_MSC_VER)
template<class I> constexpr auto make_iterator(checked_debug, I it, std::size_t size, std::size_t offset = 0) noexcept {
	return stdext::make_checked_array_iterator(it, size, offset);
}
#else
template<class I> constexpr I make_iterator(checked_debug, I it, std::size_t, std::size_t offset = 0) noexcept {
	// FIXME: does stdlib have nothing similar to msvc?
	return it + offset;
}
#endif
template<class I> constexpr I make_iterator(unchecked_debug, I it, std::size_t, std::size_t offset = 0) noexcept {
	return it + offset;
}

}  // namespace detail

template<typename buff, debug_policy debug>  //
struct iter_buffer {
	constexpr auto begin() const noexcept {
		auto b = static_cast<const buff*>(this);
		return detail::make_iterator(detail::iterator_type<debug>{}, b->data(), b->size());
	}
	constexpr auto end() const noexcept {
		auto b = static_cast<const buff*>(this);
		return detail::make_iterator(detail::iterator_type<debug>{}, b->data(), b->size(), b->size());
	}
	constexpr auto cbegin() const noexcept {
		return this->begin();
	}
	constexpr auto cend() const noexcept {
		return this->end();
	}

	constexpr auto front() const {
		auto b = static_cast<const buff*>(this);
		assert(debug == debug_policy::unchecked || !(b->empty() && "out of bound access"));
		return *(b->data());
	}
	constexpr auto back() const {
		auto b = static_cast<const buff*>(this);
		assert(debug == debug_policy::unchecked || !(b->empty() && "out of bound access"));
		return *(b->data() + b->size() - 1);
	}
	template<typename index>  // templated otherwise ambiguos overload
	constexpr auto operator[](index s) const {
		static_assert(std::is_integral<index>::value, "only integral types are valid indexes");
		static_assert(not std::is_same<index, bool>::value, "bool is not a valid index type");
		auto b = static_cast<const buff*>(this);
		assert(
		     debug == debug_policy::unchecked || (0 <= s && static_cast<decltype(b->size())>(s) < b->size() && "out of bound access"));
		return b->data()[s];
	}
};

template<class character = char,                                                        //
     conversion_policy explicit_constructor_from_charp = conversion_policy::explicit_,  //
     format_policy format = format_policy::zero_terminated,                             //
     class content_policy = default_content_policy<character>,                          //
     debug_policy debug = debug_policy::global                                          //  does not change constructors behaviour
     >
class basic_string_views
            : private create_checked_buff_view_from_str<character, explicit_constructor_from_charp, content_policy>
            , private iter_buffer<basic_string_views<character, explicit_constructor_from_charp, format, content_policy, debug>,
                   debug> {
	using crtp_access_buffer =
	     iter_buffer<basic_string_views<character, explicit_constructor_from_charp, format, content_policy, debug>, debug>;

	using base_construct = create_checked_buff_view_from_str<character, explicit_constructor_from_charp, content_policy>;

public:
	using difference_type = ptrdiff_t;
	using size_type = std::size_t;  // TODO: consider signed...
	using value_type = character;
	using pointer = const value_type*;
	using const_pointer = const value_type*;
	using reference = const value_type&;
	using const_reference = const value_type&;

	constexpr static size_type npos = static_cast<size_type>(-1);

	using base_construct::base_construct;

	/// constructor from self
	/// for all format_policy, conditional explicit
	template<typename _Dummy = void,  //
	     conversion_policy p,         //
	     format_policy f,             //
	     debug_policy d,              //
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::explicit_) && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr explicit basic_string_views(basic_string_views<character, p, f, content_policy, d> sv) noexcept(
	     base_construct::construct_noexcept) :
	            base_construct(sv.data(), sv.size()) {
		static_assert(f == format_policy::zero_terminated || f == format, "conversion to zero_terminated not allowed");
	}
	template<typename _Dummy = void,  //
	     conversion_policy p,         //
	     format_policy f,             //
	     debug_policy d,              //
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_) && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr basic_string_views(basic_string_views<character, p, f, content_policy, d> sv) noexcept(
	     base_construct::construct_noexcept) :
	            base_construct(sv.data(), sv.size()) {
		static_assert(f == format_policy::zero_terminated || f == format, "conversion to zero_terminated not allowed");
	}

	/// minimal interface
	using buff_view<character>::data;
	using buff_view<character>::size;
	using buff_view<character>::empty;

	// only for policy zero-term
	template<typename _Dummy = void,  //
	     typename std::enable_if<(format == format_policy::zero_terminated) && std::is_void<_Dummy>::value, character>::type = true>
	constexpr const_pointer c_str() const noexcept {
		return this->data();
	}

	friend crtp_access_buffer;  // otherwise static_cast in crtp does not work!
	using crtp_access_buffer::begin;
	using crtp_access_buffer::end;
	using crtp_access_buffer::cbegin;
	using crtp_access_buffer::cend;

	using crtp_access_buffer::front;
	using crtp_access_buffer::back;
	using crtp_access_buffer::operator[];

	// substring has limitation defined by the invariant.
	// string_view has no invariant
	// zstring_view can make substr only from somewhere in the middle till the end
	// non_empty_string_view should make a substring that is not empty...
	constexpr basic_string_views substr() const noexcept {
		return basic_string_views(this->data(), this->data() + this->size());
	}
	template<typename _Dummy = void,  // notice, no count!
	     typename std::enable_if<(format == format_policy::zero_terminated) && std::is_void<_Dummy>::value, character>::type = true>
	constexpr basic_string_views substr(size_type pos) const {
		return basic_string_views(this->data() + pos, this->size() - pos);
	}
	template<typename _Dummy = void,  //
	     typename std::enable_if<(format == format_policy::not_zero_terminated) && std::is_void<_Dummy>::value, character>::type = true>
	constexpr basic_string_views substr(size_type pos, size_type count = npos) const {
		return basic_string_views(this->data() + pos, std::min(this->size() - pos, count));
	}

	// for string_view pop_back works on non-empty
	// for zstring_view pop_back never works
	// for non_empty_string_view pop_back works on size >=2
};

}  // namespace string_views
