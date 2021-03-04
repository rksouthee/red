#ifndef RED_GAP_BUFFER_H
#define RED_GAP_BUFFER_H

#include <cstddef>
#include <iterator>

class Gap_buffer {
public:
	using value_type = char;
	using reference = value_type&;
	using size_type = std::size_t;
	class iterator;

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

	class iterator {
	public:
		using value_type = char;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::random_access_iterator_tag;

	private:
		friend class Gap_buffer;
		char* ptr;
		char* gap_begin;
		char* gap_end;

	public:
		iterator() = default;
		iterator(char* ptr, char* gap_begin, char* gap_end);

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
		iterator& operator+=(difference_type n);
		friend iterator operator+(iterator x, difference_type n);

		reference operator[](difference_type n) const;

		iterator& operator--();
		iterator operator--(int);
		iterator& operator-=(difference_type n);
		friend iterator operator-(iterator x, difference_type n);
		friend difference_type operator-(const iterator& x, const iterator& y);
	};

	iterator begin();
	iterator end();

	void reserve(size_type n);

	void insert(iterator i, size_type n, char c);
	void erase(iterator i, size_type n);
	iterator erase(iterator f, iterator l);
};

#endif
