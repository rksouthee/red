#pragma once

#include <cstddef>

class Gap_buffer {
public:
	using value_type = char;
	using size_type = std::size_t;

private:
	char* data_begin = nullptr;
	char* data_end = nullptr;
	char* gap_begin = nullptr;
	char* gap_end = nullptr;

public:
	~Gap_buffer();
	Gap_buffer() = default;
	Gap_buffer(const Gap_buffer& x);
	Gap_buffer& operator=(const Gap_buffer& x);
	Gap_buffer(Gap_buffer&& x) noexcept;
	Gap_buffer& operator=(Gap_buffer&& x) noexcept;

	friend bool operator==(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator!=(const Gap_buffer& x, const Gap_buffer& y);

	friend bool operator <(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator >(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator<=(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator>=(const Gap_buffer& x, const Gap_buffer& y);

	size_type capacity() const;
	size_type size() const;

	const char* begin0() const;
	const char* end0() const;
	const char* begin1() const;
	const char* end1() const;
};
