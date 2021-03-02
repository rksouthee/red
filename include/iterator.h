#ifndef RED_ITERATOR_H
#define RED_ITERATOR_H

#include <cstddef>
#include <iterator>
#include "gap_buffer.h"

/*
 * Stable iterator to a Gap_buffer
 */
struct Indexed_iterator {
	using value_type = char;
	using reference = value_type&;
	using pointer = value_type*;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::bidirectional_iterator_tag; // TODO: Use random_access_iterator_tag?

	Gap_buffer* data;
	Gap_buffer::size_type index;

	Indexed_iterator() = default;
	Indexed_iterator(Gap_buffer& data, Gap_buffer::size_type index);

	friend bool operator==(const Indexed_iterator& x, const Indexed_iterator& y);
	friend bool operator!=(const Indexed_iterator& x, const Indexed_iterator& y);

	friend bool operator <(const Indexed_iterator& x, const Indexed_iterator& y);
	friend bool operator >(const Indexed_iterator& x, const Indexed_iterator& y);
	friend bool operator<=(const Indexed_iterator& x, const Indexed_iterator& y);
	friend bool operator>=(const Indexed_iterator& x, const Indexed_iterator& y);

	reference operator*() const;
	pointer operator->() const;

	Indexed_iterator& operator++();
	Indexed_iterator operator++(int);

	Indexed_iterator& operator--();
	Indexed_iterator operator--(int);
};

#endif
