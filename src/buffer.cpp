#include "buffer.h"

Buffer::iterator Buffer::begin()
{
	return contents.begin();
}

Buffer::iterator Buffer::end()
{
	return contents.end();
}

bool Buffer::write_file(HANDLE file_handle)
{
	bool result = false;
	const char* data = contents.begin0();
	DWORD bytes = static_cast<DWORD>(contents.end0() - data);
	DWORD bytes_written;
	if (WriteFile(file_handle, data, bytes, &bytes_written, NULL) && bytes_written == bytes) {
		data = contents.begin1();
		bytes = static_cast<DWORD>(contents.end1() - data);
		if (WriteFile(file_handle, data, bytes, &bytes_written, NULL) && bytes_written == bytes) {
			modified = false;
			result = true;
		} else {
		}
	} else {
	}
	return result;
}

void Buffer::insert(iterator i, char c)
{
	modified = true;
	contents.insert(i, 1, c);
}

void Buffer::erase(iterator i)
{
	modified = true;
	contents.erase(i, 1);
}

void Buffer::erase(iterator f, iterator l)
{
	modified = true;
	contents.erase(f, l);
}
