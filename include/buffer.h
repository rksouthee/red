#ifndef RED_BUFFER_H
#define RED_BUFFER_H

#include <windows.h>
#include <string>
#include "gap_buffer.h"
#include "iterator.h"

struct Buffer {
	using Buffer_storage = Gap_buffer;
	using size_type = Buffer_storage::size_type;
	using iterator = Indexed_iterator;

	std::string name;
	Buffer_storage contents;
	bool modified;

	bool write_file(HANDLE file_handle);

	iterator begin();
	iterator end();

	void insert(iterator i, char c);
	void erase(iterator i);
	iterator erase(iterator f, iterator l);
};

#endif
