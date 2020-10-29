#pragma once

#include <windows.h>
#include <cstddef>
#include <iterator>
#include <string>

/*
 * We should probably factor our a String_buffer in this example for future cases where
 * we would like to switch the underlying storage, for example using a gap buffer or a line
 * list.
 */

class Buffer {
public:
	using size_type = std::string::size_type;
	struct iterator;

private:
	const char* name; // TODO: use std::string_view instead?
	std::string contents;
	bool modified_ = false;

public:
	Buffer() = default;
	Buffer(const char* name, std::string contents);

	// TODO: Do we need this? Used for saving the buffer
	const char* data() const;
	const char* filename() const;
	size_type size() const;

	bool modified() const;

	bool write_file(HANDLE file_handle);

	struct iterator {
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

	iterator begin();
	iterator end();

	void insert(iterator i, char c);
	void erase(iterator i);
};
