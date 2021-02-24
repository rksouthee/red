#include "buffer.h"

Buffer::Buffer(std::string name, Buffer::Buffer_storage contents) :
	name(std::move(name)),
	contents(std::move(contents))
{
}

const std::string& Buffer::filename() const
{
	return name;
}

void Buffer::filename(std::string name)
{
	this->name = std::move(name);
}

Buffer::iterator Buffer::begin()
{
	return contents.begin();
}

Buffer::iterator Buffer::end()
{
	return contents.end();
}

bool Buffer::modified() const
{
	return modified_;
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
			modified_ = false;
			result = true;
		} else {
		}
	} else {
	}
	return result;
}

void Buffer::insert(iterator i, char c)
{
	modified_ = true;
	contents.insert(i, 1, c);
}

void Buffer::erase(iterator i)
{
	modified_ = true;
	contents.erase(i, 1);
}
