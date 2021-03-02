#include "iterator.h"
#include <cassert>

Indexed_iterator::Indexed_iterator(Gap_buffer& data, Gap_buffer::size_type index) :
	data(&data),
	index(index)
{
}

bool operator==(const Indexed_iterator& x, const Indexed_iterator& y)
{
	assert(x.data == y.data);
	return x.index == y.index;
}

bool operator!=(const Indexed_iterator& x, const Indexed_iterator& y)
{
	return !(x == y);
}

bool operator <(const Indexed_iterator& x, const Indexed_iterator& y)
{
	assert(x.data == y.data);
	return x.index < y.index;
}

bool operator >(const Indexed_iterator& x, const Indexed_iterator& y)
{
	return y < x;
}

bool operator<=(const Indexed_iterator& x, const Indexed_iterator& y)
{
	return !(y < x);
}

bool operator>=(const Indexed_iterator& x, const Indexed_iterator& y)
{
	return !(x < y);
}

Indexed_iterator::reference Indexed_iterator::operator*() const
{
	return (*data)[index];
}

Indexed_iterator::pointer Indexed_iterator::operator->() const
{
	return &**this;
}

Indexed_iterator& Indexed_iterator::operator++()
{
	++index;
	return *this;
}

Indexed_iterator Indexed_iterator::operator++(int)
{
	Indexed_iterator tmp = *this;
	++*this;
	return tmp;
}

Indexed_iterator& Indexed_iterator::operator--()
{
	--index;
	return *this;
}

Indexed_iterator Indexed_iterator::operator--(int)
{
	Indexed_iterator tmp = *this;
	--*this;
	return tmp;
}

