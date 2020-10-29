#include "gap_buffer.hpp"

#include <algorithm>
#include <new>

Gap_buffer::~Gap_buffer()
{
	delete[] data_begin;
}

Gap_buffer::Gap_buffer(const Gap_buffer& x) :
	data_begin(new char[x.size()])
{
	gap_begin = std::copy(x.begin0(), x.end0(), data_begin);
	gap_end = gap_begin;
	data_end = std::copy(x.begin1(), x.end1(), gap_end);
}

Gap_buffer& Gap_buffer::operator=(const Gap_buffer& x)
{
	if (capacity() < x.size()) {
		Gap_buffer tmp = x;
		std::swap(*this, tmp);
	} else {
		gap_begin = std::copy(x.begin0(), x.end0(), data_begin);
		gap_end = std::copy_backward(x.begin1(), x.end1(), data_end);
	}
	return *this;
}

Gap_buffer::Gap_buffer(Gap_buffer&& x) noexcept :
	data_begin(x.data_begin),
	data_end(x.data_end),
	gap_begin(x.gap_begin),
	gap_end(x.gap_end)
{
	x.data_begin = nullptr;
	x.data_end = nullptr;
}

Gap_buffer& Gap_buffer::operator=(Gap_buffer&& x) noexcept
{
	this->~Gap_buffer();
	::new (static_cast<void*>(this)) Gap_buffer(std::move(x));
	return *this;
}

bool operator==(const Gap_buffer& x, const Gap_buffer& y)
{
	if (x.size() != y.size())
		return false;

	const char* f_x = x.begin0();
	const char* f_y = y.begin0();

	while (true) {
		if (f_x == x.end0()) {
			f_x = x.begin1();
			while (f_y != y.end0()) {
				if (*f_x != *f_y)
					return false;
				++f_x;
				++f_y;
			}
			return std::equal(f_x, x.end1(), y.begin1());
		}

		if (f_y == y.end0()) {
			f_y = y.begin1();
			do {
				if (*f_x != *f_y)
					return false;
				++f_x;
				++f_y;
			} while (f_x != x.end0());
			return std::equal(x.begin1(), x.end1(), f_y);
		}

		if (*f_x != *f_y)
			return false;
		++f_x;
		++f_y;
	}
}

bool operator!=(const Gap_buffer& x, const Gap_buffer& y)
{
	return !(x == y);
}

bool operator <(const Gap_buffer& x, const Gap_buffer& y)
{
	const char* f_x = x.begin0();
	const char* f_y = y.begin0();

	while (true) {
		if (f_x == x.end0()) {
			f_x = x.begin1();
			while (f_y != y.end0()) {
				if (f_x == x.end1())
					return true;
				if (*f_x < *f_y)
					return true;
				if (*f_y < *f_x)
					return false;
				++f_x;
				++f_y;
			}
			return std::lexicographical_compare(f_x, x.end1(), y.begin1(), y.end1());
		}

		if (f_y == y.end0()) {
			f_y = y.begin1();
			do {
				if (f_y == y.end1())
					return false;
				if (*f_x < *f_y)
					return true;
				if (*f_y < *f_x)
					return false;
				++f_x;
				++f_y;
			} while (f_x != x.end0());
			return std::lexicographical_compare(x.begin1(), x.end1(), f_y, y.end1());
		}

		if (*f_x < *f_y)
			return true;
		if (*f_y < *f_x)
			return false;
		++f_x;
		++f_y;
	}
}

bool operator >(const Gap_buffer& x, const Gap_buffer& y)
{
	return y < x;
}

bool operator<=(const Gap_buffer& x, const Gap_buffer& y)
{
	return !(y < x);
}

bool operator>=(const Gap_buffer& x, const Gap_buffer& y)
{
	return !(x < y);
}

Gap_buffer::size_type Gap_buffer::capacity() const
{
	return data_end - data_begin;
}

Gap_buffer::size_type Gap_buffer::size() const
{
	return (gap_begin - data_begin) + (data_end - gap_end);
}

const char* Gap_buffer::begin0() const
{
	return data_begin;
}

const char* Gap_buffer::end0() const
{
	return gap_begin;
}

const char* Gap_buffer::begin1() const
{
	return gap_end;
}

const char* Gap_buffer::end1() const
{
	return data_end;
}
