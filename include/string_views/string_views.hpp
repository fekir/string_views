#pragma once

#include <iterator>

#include <cstddef>      // ptrdiff
#include <type_traits>  // std::enable_if, std::integral_constant
#include <cassert>

namespace string_views {

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
}; # FIXME: implementation is incomplete

// constexpr strlen
namespace detail {
template<class T> long constexpr len(const T* str) {
	return *str ? 1 + len(++str) : 0;
}

template<enum debug_policy p> using iterator_type = std::integral_constant<debug_policy, p>;
using unchecked_debug = iterator_type<debug_policy::unchecked>;
using checked_debug = iterator_type<debug_policy::checked>;

// FIXME: add conversion tests and that it helps to catch bugs
// could save checked iter instead of pointer-type?
// as downside c_str/data gets more complicated...
#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL >= 1 && defined(_MSC_VER)
template<class I> constexpr auto make_iterator(checked_debug, I it, std::size_t size, std::size_t offset = 0) {
	return stdext::make_checked_array_iterator(it, size, offset);
}
#else
template<class I> constexpr I make_iterator(checked_debug, I it, std::size_t, std::size_t offset = 0) {
	// FIXME: does stdlib have nothing similar to msvc?
	return it + offset;
}
#endif
template<class I> constexpr I make_iterator(unchecked_debug, I it, std::size_t, std::size_t offset = 0) {
	return it + offset;
}

}  // namespace detail

template<class character> struct default_content_policy {
	static constexpr void check(const character* begin, const character* end) noexcept {
		(void) begin;
		(void) end;
	}
};

template<class character = char,                                                        //
     conversion_policy explicit_constructor_from_charp = conversion_policy::explicit_,  //
     format_policy format = format_policy::zero_terminated,                             //
     class content_policy = default_content_policy<character>,                          //
     debug_policy debug = debug_policy::global                                          //  does not change constructors behaviour
     >
struct basic_string_views {
	using difference_type = ptrdiff_t;
	using size_type = std::size_t;  // TODO: consider signed...
	using value_type = character;
	using pointer = const value_type*;
	using const_pointer = const value_type*;
	using reference = const value_type&;
	using const_reference = const value_type&;

	/// constructor from char*
	/// for all policies, and conditional explicit
	template<typename _Dummy = void,
		 typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::explicit_)
				   //&& (format == format_policy::zero_terminated)  //
				   && std::is_void<_Dummy>::value,
			  character>::type = false>
	constexpr explicit basic_string_views(const value_type* data) : m_begin{data}, m_end{data + detail::len(data)} {
		content_policy::check(m_begin, m_end);
	}

	template<typename _Dummy = void,
		 typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_)
				   //&& (format == format_policy::zero_terminated)  //
				   && std::is_void<_Dummy>::value,
			  character>::type = true>
	constexpr basic_string_views(const value_type* data) : m_begin{data}, m_end{data + detail::len(data)} {
		content_policy::check(m_begin, m_end);
	}

	/// constructor from iterators
	/// for all format_policy, conditional explicit
	constexpr explicit basic_string_views(const value_type* begin, const value_type* end) : m_begin{begin}, m_end{end} {
		content_policy::check(m_begin, m_end);
	}
	constexpr explicit basic_string_views(const value_type* begin, size_type size) : m_begin{begin}, m_end{begin + size} {
		content_policy::check(m_begin, m_end);
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
				m_begin{sv.begin()}, m_end{sv.end()} {
		static_assert(f == format_policy::zero_terminated || f == format, "not compatible format policy");
		content_policy::check(m_begin, m_end);
	}
	template<typename _Dummy = void,  //
		 conversion_policy p,         //
		 format_policy f,             //
		 debug_policy d,              //
		 typename std::enable_if<(explicit_constructor_from_charp == conversion_policy::implicit_) && std::is_void<_Dummy>::value,
			  character>::type = true>
	constexpr basic_string_views(basic_string_views<character, p, f, content_policy, d> sv) : m_begin{sv.begin()}, m_end{sv.end()} {
		static_assert(f == format_policy::zero_terminated || f == format, "not compatible format policy");
		content_policy::check(m_begin, m_end);
	}

	/// constructor from std string object - does not work for string_view
	/// for all format_policy, conditional explicit
	template<class string,
		 typename std::enable_if<explicit_constructor_from_charp == conversion_policy::explicit_ && !std::is_void<string>::value,
			  character>::type = true>
	constexpr explicit basic_string_views(const string& s) : m_begin{s.c_str()}, m_end{s.c_str() + s.size()} {
		content_policy::check(m_begin, m_end);
	}
	template<class string,
		 typename std::enable_if<explicit_constructor_from_charp == conversion_policy::implicit_ && !std::is_void<string>::value,
			  character>::type = true>
	constexpr basic_string_views(const string& s) : m_begin{s.c_str()}, m_end{s.c_str() + s.size()} {
		content_policy::check(m_begin, m_end);
	}

	/// accessor functions
	constexpr const_pointer data() const noexcept {
		return this->m_begin;
	}
	// only for policy zero-term
	template<typename _Dummy = void,  //
		 typename std::enable_if<(format == format_policy::zero_terminated) && std::is_void<_Dummy>::value, character>::type = true>
	constexpr const_pointer c_str() const noexcept {
		return this->m_begin;
	}
	constexpr const_reference front() const {
		assert(debug == debug_policy::unchecked || !(this->empty() && "out of bound access"));
		return *(this->m_begin);
	}
	constexpr const_reference back() const {
		assert(debug == debug_policy::unchecked || !(this->empty() && "out of bound access"));
		return *(this->m_end - 1);
	}

	/// iterators
	constexpr auto begin() const noexcept {
		return detail::make_iterator(detail::iterator_type<debug>{}, this->m_begin, this->size());
	}
	constexpr auto end() const noexcept {
		return detail::make_iterator(detail::iterator_type<debug>{}, this->m_begin, this->size(), this->size());
	}
	constexpr auto cbegin() const noexcept {
		return detail::make_iterator(detail::iterator_type<debug>{}, this->m_begin, this->size());
	}
	constexpr auto cend() const noexcept {
		return detail::make_iterator(detail::iterator_type<debug>{}, this->m_begin, this->size(), this->size());
	}
	// safe/implicit only for 0-terminated, do we need it?
	/*	explicit operator const_pointer() {
		return this->m_begin;
	}*/

	// FIXME: add overloads for integral types, and disallow enum, bool, characters, ...
	constexpr const_reference operator[](size_type s) const {
		assert(debug == debug_policy::unchecked || (s >= 0 && s < this->size() && "out of bound access"));
		return this->m_begin[s];
	}
	// reverse iterators

	// FIXME
	// substring has limitation defined by the invariant.
	// string_view has no invariant
	// zstring_view can make substr only from somewhere in the middle till the end
	// non_empty_string_view should make a substring that is not empty...
	// otherwise make this method generic, and throught template param make
	basic_string_views substring() {
		return basic_string_views(this->m_begin, this->m_end);
	}

	constexpr size_type size() const noexcept {
		return static_cast<size_type>(this->m_end - this->m_begin);
	}
	constexpr bool empty() const noexcept {
		return this->m_end == this->m_begin;
	}

	// for string_view pop_back works on non-empty
	// for zstring_view pop_back never works
	// for non_empty_string_view pop_back works on size >=2

private:
	const_pointer m_begin;
	const_pointer m_end; // TODO: consider storing length
};

}  // namespace string_views
