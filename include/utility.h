#ifndef RED_UTILITY_H
#define RED_UTILITY_H

#include <iterator>

/*
 * XXX: Assuming codepage 850, should correctly use locales and switch to utf-8
 */
inline bool is_print(char character)
{
	auto uchar = static_cast<unsigned char>(character);
	return uchar == '\t' || (uchar >= ' ' && uchar <= '~') || (uchar >= 128 && uchar <= 254);
}

template <typename I>
// requires BidirectionalIterator(I)
I find_backward(I f, I l, const typename std::iterator_traits<I>::value_type& x)
{
	while (l != f) {
		--l;
		if (*l == x)
			return std::next(l);
	}
	return l;
}

#endif
