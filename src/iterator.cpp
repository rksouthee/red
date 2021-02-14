#include <cassert>

String_iterator::iterator(std::string& data, std::string::size_type index) :
	data(&data),
	index(index)
{
}

bool operator==(const String_iterator& x, const iterator& y)
{
	assert(x.data == y.data);
	return x.index == y.index;
}

bool operator!=(const String_iterator& x, const iterator& y)
{
	return !(x == y);
}

bool operator <(const String_iterator& x, const iterator& y)
{
	assert(x.data == y.data);
	return x.index < y.index;
}

bool operator >(const String_iterator& x, const iterator& y)
{
	return y < x;
}

bool operator<=(const String_iterator& x, const iterator& y)
{
	return !(y < x);
}

bool operator>=(const String_iterator& x, const iterator& y)
{
	return !(x < y);
}

String_iterator::reference iterator::operator*() const
{
	return (*data)[index];
}

String_iterator::pointer iterator::operator->() const
{
	return &**this;
}

String_iterator& iterator::operator++()
{
	++index;
	return *this;
}

String_iterator iterator::operator++(int)
{
	String_iterator tmp = *this;
	++*this;
	return tmp;
}

String_iterator& iterator::operator--()
{
	--index;
	return *this;
}

String_iterator iterator::operator--(int)
{
	String_iterator tmp = *this;
	--*this;
	return tmp;
}

