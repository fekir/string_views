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
struct checked_buff_view : private buff_view<character> {
	constexpr checked_buff_view(const character* begin_, std::size_t size_) : buff_view<character>(begin_, size_) {
		content_policy::check(this->data(), this->data() + this->size());
	}
	using buff_view<character>::data;
	using buff_view<character>::size;
	using buff_view<character>::empty;
};

namespace detail {
template<typename character> unsigned long constexpr len(const character* str) {
	return *str ? 1u + len(++str) : 0u;
}
}  // namespace detail
// should this apply to both const char* and std::string? what about operator const char*?
enum class conversion_policy {
	implicit_,
	explicit_,
};  // FIXME: find better name as explicit reserved

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
	//constexpr auto operator[](typename buff::size_type s) const { // FIXME: does not work....
	constexpr auto operator[](std::size_t s) const {
		auto b = static_cast<const buff*>(this);
		assert(debug == debug_policy::unchecked || (s < b->size() && "out of bound access"));
		return b->data()[s];
	}
	// FIXME: add overloads for integral types, and disallow enum, bool, characters, ...
	//constexpr auto operator[](bool) const = delete;
	// causes issue, as 0 is neither bool nor std::size_t, but convertible to both
};

template<class character = char,                                                        //
     conversion_policy explicit_constructor_from_charp = conversion_policy::explicit_,  //
     format_policy format = format_policy::zero_terminated,                             //
     class content_policy = default_content_policy<character>,                          //
     debug_policy debug = debug_policy::global                                          //  does not change constructors behaviour
     >
struct basic_string_views
            : private checked_buff_view<character, content_policy>
            , private iter_buffer<basic_string_views<character, explicit_constructor_from_charp, format, content_policy, debug>,
                   debug> {
	using difference_type = ptrdiff_t;
	using size_type = std::size_t;  // TODO: consider signed...
	using value_type = character;
	using pointer = const value_type*;
	using const_pointer = const value_type*;
	using reference = const value_type&;
	using const_reference = const value_type&;

	using crtp_access_buffer =
	     iter_buffer<basic_string_views<character, explicit_constructor_from_charp, format, content_policy, debug>, debug>;

	/// constructor from char*
	/// for all policies, and conditional explicit
	template<typename _Dummy = void,
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::explicit_)
	               //&& (format == format_policy::zero_terminated)  //
	               && std::is_void<_Dummy>::value,
	          character>::type = false>
	constexpr explicit basic_string_views(const value_type* data_) :
	            checked_buff_view<character, content_policy>(data_, detail::len(data_)) {
	}

	constexpr basic_string_views(std::nullptr_t) = delete;

	template<typename _Dummy = void,
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_)
	               //&& (format == format_policy::zero_terminated)  //
	               && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr basic_string_views(const value_type* data_) : checked_buff_view<character, content_policy>(data_, detail::len(data_)) {
	}

	/// constructor from iterators
	/// for all format_policy, conditional explicit
	constexpr basic_string_views(const value_type* begin_, const value_type* end_) :
	            checked_buff_view<character, content_policy>(begin_, end_ - begin_) {
	}
	constexpr basic_string_views(const value_type* begin_, size_type size_) :
	            checked_buff_view<character, content_policy>(begin_, size_) {
	}

	/// constructor from self
	/// for all format_policy, conditional explicit
	template<typename _Dummy = void,  //
	     conversion_policy p,         //
	     format_policy f,             //
	     debug_policy d,              //
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::explicit_) && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr explicit basic_string_views(basic_string_views<character, p, f, content_policy, d> sv) :
	            checked_buff_view<character, content_policy>(sv.data(), sv.size()) {
		static_assert(f == format_policy::zero_terminated || f == format, "not compatible format policy");
	}
	template<typename _Dummy = void,  //
	     conversion_policy p,         //
	     format_policy f,             //
	     debug_policy d,              //
	     typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_) && std::is_void<_Dummy>::value,
	          character>::type = true>
	constexpr basic_string_views(basic_string_views<character, p, f, content_policy, d> sv) :
	            checked_buff_view<character, content_policy>(sv.data(), sv.size()) {
		static_assert(f == format_policy::zero_terminated || f == format, "not compatible format policy");
	}

	/// constructor from std string object - does not work for string_view
	/// for all format_policy, conditional explicit
	template<class string,
	     typename std::enable_if<explicit_constructor_from_charp == conversion_policy::explicit_ && !std::is_void<string>::value,
	          character>::type = true>
	constexpr explicit basic_string_views(const string& s) : checked_buff_view<character, content_policy>(s.c_str(), s.size()) {
	}
	template<class string,
	     typename std::enable_if<explicit_constructor_from_charp == conversion_policy::implicit_ && !std::is_void<string>::value,
	          character>::type = true>
	constexpr basic_string_views(const string& s) : checked_buff_view<character, content_policy>(s.c_str(), s.size()) {
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

	// safe/implicit only for 0-terminated, do we need it?
	/*	explicit operator const_pointer() {
		return this->m_begin;
	}*/

	// FIXME
	// substring has limitation defined by the invariant.
	// string_view has no invariant
	// zstring_view can make substr only from somewhere in the middle till the end
	// non_empty_string_view should make a substring that is not empty...
	// otherwise make this method generic, and throught template param make
	basic_string_views substring() {
		return basic_string_views(this->data(), this->data() + this->size());
	}

	// for string_view pop_back works on non-empty
	// for zstring_view pop_back never works
	// for non_empty_string_view pop_back works on size >=2
};

}  // namespace string_views
