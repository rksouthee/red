#include "buffer.hpp"
#include <cassert>

Buffer::Buffer(const char* name, std::string contents) :
	name(name),
	contents(std::move(contents))
{
}

Buffer::iterator::iterator(std::string& data, std::string::size_type index) :
	data(&data),
	index(index)
{
}

const char* Buffer::data() const
{
	return contents.data();
}

const char* Buffer::filename() const
{
	return name;
}

Buffer::size_type Buffer::size() const
{
	return contents.size();
}

bool operator==(const Buffer::iterator& x, const Buffer::iterator& y)
{
	assert(x.data == y.data);
	return x.index == y.index;
}

bool operator!=(const Buffer::iterator& x, const Buffer::iterator& y)
{
	return !(x == y);
}

bool operator <(const Buffer::iterator& x, const Buffer::iterator& y)
{
	assert(x.data == y.data);
	return x.index < y.index;
}

bool operator >(const Buffer::iterator& x, const Buffer::iterator& y)
{
	return y < x;
}

bool operator<=(const Buffer::iterator& x, const Buffer::iterator& y)
{
	return !(y < x);
}

bool operator>=(const Buffer::iterator& x, const Buffer::iterator& y)
{
	return !(x < y);
}

Buffer::iterator::reference Buffer::iterator::operator*() const
{
	return (*data)[index];
}

Buffer::iterator::pointer Buffer::iterator::operator->() const
{
	return &**this;
}

Buffer::iterator& Buffer::iterator::operator++()
{
	++index;
	return *this;
}

Buffer::iterator Buffer::iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}

Buffer::iterator& Buffer::iterator::operator--()
{
	--index;
	return *this;
}

Buffer::iterator Buffer::iterator::operator--(int)
{
	iterator tmp = *this;
	--*this;
	return tmp;
}

Buffer::iterator Buffer::begin()
{
	return iterator(contents, 0);
}

Buffer::iterator Buffer::end()
{
	return iterator(contents, contents.size());
}

bool Buffer::modified() const
{
	return modified_;
}

bool Buffer::write_file(HANDLE file_handle)
{
	DWORD bytes = static_cast<DWORD>(contents.size());
	DWORD bytes_written;
	if (WriteFile(file_handle, contents.data(), bytes, &bytes_written, NULL) &&
	    bytes_written == bytes) {
		modified_ = false;
		return true;
	}
	return false;
}

void Buffer::insert(iterator i, char c)
{
	modified_ = true;
	contents.insert(i.index, 1, c);
}

void Buffer::erase(iterator i)
{
	modified_ = true;
	contents.erase(i.index, 1);
}
