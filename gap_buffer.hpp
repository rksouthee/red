#pragma once

#include <cstddef>
#include <iterator>

class Gap_buffer {
public:
	using value_type = char;
	using reference = value_type&;
	using size_type = std::size_t;
	struct iterator;

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

	Gap_buffer(size_type n, char c);

	friend bool operator==(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator!=(const Gap_buffer& x, const Gap_buffer& y);

	friend bool operator <(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator >(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator<=(const Gap_buffer& x, const Gap_buffer& y);
	friend bool operator>=(const Gap_buffer& x, const Gap_buffer& y);

	size_type capacity() const;
	size_type size() const;

	reference operator[](size_type i);

	const char* begin0() const;
	const char* end0() const;
	const char* begin1() const;
	const char* end1() const;

	struct iterator {
		using value_type = char;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;

		Gap_buffer* buffer;
		Gap_buffer::size_type index;

		iterator() = default;

		iterator(Gap_buffer& buffer, Gap_buffer::size_type index);

		friend bool operator==(const iterator& x, const iterator& y);
		friend bool operator!=(const iterator& x, const iterator& y);

		friend bool operator <(const iterator& x, const iterator& y);
		friend bool operator >(const iterator& x, const iterator& y);
		friend bool operator<=(const iterator& x, const iterator& y);
		friend bool operator>=(const iterator& x, const iterator& y);

		reference operator*() const;
		pointer operator->() const;

		iterator& operator++();
		iterator operator++(int);

		iterator& operator--();
		iterator operator--(int);
	};

	iterator begin();
	iterator end();

	void reserve(size_type n);

	void insert(iterator i, size_type n, char c);
	void erase(iterator i, size_type n);

private:
	void gap_move(iterator i);
};
