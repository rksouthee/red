#include "gap_buffer.h"

#include <algorithm>
#include <new>
#include <tuple>

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

Gap_buffer::Gap_buffer(size_type n, char c) :
	data_begin(new char[n])
{
	gap_begin = std::fill_n(data_begin, n, c);
	gap_end = gap_begin;
	data_end = gap_end;
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

Gap_buffer::reference Gap_buffer::operator[](size_type i)
{
	size_type n = gap_begin - data_begin;
	if (i >= n)
		i += (gap_end - gap_begin);
	return data_begin[i];
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

Gap_buffer::iterator::iterator(Gap_buffer& buffer, Gap_buffer::size_type index) :
	buffer(&buffer),
	index(index)
{
}

bool operator==(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return x.index == y.index;
}

bool operator!=(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return !(x == y);
}

bool operator <(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return x.index < y.index;
}

bool operator >(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return y < x;
}

bool operator<=(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return !(y < x);
}

bool operator>=(const Gap_buffer::iterator& x, const Gap_buffer::iterator& y)
{
	return !(x < y);
}

Gap_buffer::iterator::reference Gap_buffer::iterator::operator*() const
{
	return (*buffer)[index];
}

Gap_buffer::iterator::pointer Gap_buffer::iterator::operator->() const
{
	return &**this;
}

Gap_buffer::iterator& Gap_buffer::iterator::operator++()
{
	++index;
	return *this;
}

Gap_buffer::iterator Gap_buffer::iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}

Gap_buffer::iterator& Gap_buffer::iterator::operator--()
{
	--index;
	return *this;
}

Gap_buffer::iterator Gap_buffer::iterator::operator--(int)
{
	iterator tmp = *this;
	--*this;
	return tmp;
}

Gap_buffer::iterator Gap_buffer::begin()
{
	return iterator(*this, 0);
}

Gap_buffer::iterator Gap_buffer::end()
{
	return iterator(*this, size());
}

void Gap_buffer::reserve(size_type n)
{
	if (n <= capacity())
		return;

	char* new_data_begin = new char[n];
	char* new_data_end = new_data_begin + n;
	char* new_gap_begin = std::copy(begin0(), end0(), new_data_begin);
	char* new_gap_end = std::copy_backward(begin1(), end1(), new_data_end);

	delete[] data_begin;

	data_begin = new_data_begin;
	data_end = new_data_end;
	gap_begin = new_gap_begin;
	gap_end = new_gap_end;
}

template <typename I0, typename I1, typename N>
std::pair<I0, I1> move_backward_n(I0 l0, N n, I1 l1)
{
	while (n) {
		*--l1 = *--l0;
		--n;
	}
	return std::make_pair(l0, l1);
}

template <typename I0, typename I1, typename N>
std::pair<I0, I1> move_n(I0 f0, N n, I1 f1)
{
	while (n) {
		*f1++ = *f0++;
		--n;
	}
	return std::make_pair(f0, f1);
}

void Gap_buffer::gap_move(iterator i)
{
	size_type index = i.index;
	size_type n = gap_begin - data_begin;
	if (index < n)
		std::tie(gap_begin, gap_end) = move_backward_n(gap_begin, n - index, gap_end);
	else
		std::tie(gap_end, gap_begin) = move_n(gap_end, index - n, gap_begin);
}

void Gap_buffer::insert(iterator i, size_type n, char c)
{
	if (static_cast<size_type>(gap_end - gap_begin) < n)
		reserve(size() + n);
	gap_move(i);
	gap_begin = std::fill_n(gap_begin, n, c);
}

void Gap_buffer::erase(iterator i, size_type n)
{
	gap_move(i);
	gap_end += n;
}

void Gap_buffer::erase(iterator f, iterator l)
{
	gap_move(f);
	gap_end += (l.index - f.index);
}
