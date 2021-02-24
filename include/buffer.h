#ifndef RED_BUFFER_H
#define RED_BUFFER_H

#include <windows.h>
#include <string>
#include "gap_buffer.h"

/*
 * We should probably factor our a String_buffer in this example for future cases where
 * we would like to switch the underlying storage, for example using a gap buffer or a line
 * list.
 */

class Buffer {
private:
	using Buffer_storage = Gap_buffer;

public:
	using size_type = Buffer_storage::size_type;
	using iterator = Buffer_storage::iterator;

private:
	std::string name;
	Buffer_storage contents;
	bool modified_ = false;

public:
	Buffer() = default;
	Buffer(std::string name, Buffer_storage contents);

	const std::string& filename() const;

	bool modified() const;

	bool write_file(HANDLE file_handle);

	iterator begin();
	iterator end();

	void insert(iterator i, char c);
	void erase(iterator i);
};

#endif
