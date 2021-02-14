#ifndef RED_ITERATOR_H
#define RED_ITERATOR_H

#include <cstddef>
#include <iterator>
#include <string>

/*
 * Stable string iterator
 */
struct String_iterator {
	using value_type = char;
	using reference = value_type&;
	using pointer = value_type*;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::bidirectional_iterator_tag;

	std::string* data;
	std::string::size_type index;

	iterator() = default;
	iterator(std::string& data, std::string::size_type index);

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

#endif
