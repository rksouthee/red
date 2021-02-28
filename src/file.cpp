#include "file.h"
#include <cassert>

DWORD file_open(std::string filename, Buffer& buffer)
{
	DWORD last_error = 0;
	HANDLE file_handle = CreateFileA(filename.c_str(),
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 OPEN_EXISTING,
					 FILE_ATTRIBUTE_NORMAL,
					 0);
	if (file_handle != INVALID_HANDLE_VALUE) {
		// TODO: Use GetFileSizeEx instead
		DWORD file_size = GetFileSize(file_handle, NULL);
		if (file_size != INVALID_FILE_SIZE) {
			DWORD bytes_read;
			Gap_buffer temp_buffer(std::size_t(file_size), 0);
			// TODO: Check we read the correct number of bytes
			if (ReadFile(file_handle, &temp_buffer[0], file_size, &bytes_read, NULL)) {
				buffer = Buffer{std::move(filename), std::move(temp_buffer), false};
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
		CloseHandle(file_handle);
	} else {
		last_error = GetLastError();
		if (last_error == ERROR_FILE_NOT_FOUND) {
			last_error = 0;
			buffer = Buffer{std::move(filename), Gap_buffer{}, false};
		}
	}
	return last_error;
}

/*
 * Try to safely save the file by writing to a temporary file and if that succeeds
 * move the file.
 */
DWORD file_save(Buffer& buffer)
{
	assert(!buffer.name.empty());
	DWORD last_error = 0;
	char temp_path[MAX_PATH];
	DWORD length = GetTempPathA(MAX_PATH, temp_path);
	if (length > 0 && length <= MAX_PATH) {
		char temp_filename[MAX_PATH];
		if (GetTempFileNameA(temp_path, "RED", 0, temp_filename) != 0) {
			HANDLE temp_file_handle = CreateFileA(temp_filename,
							      GENERIC_WRITE,
							      0,
							      NULL,
							      CREATE_ALWAYS,
							      FILE_ATTRIBUTE_NORMAL,
							      NULL);
			if (temp_file_handle != INVALID_HANDLE_VALUE) {
				if (buffer.write_file(temp_file_handle)) {
					CloseHandle(temp_file_handle);
					if (MoveFileEx(temp_filename,
						       buffer.name.c_str(),
						       MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
					} else {
						last_error = GetLastError();
					}
				} else {
					last_error = GetLastError();
					CloseHandle(temp_file_handle);
				}
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
	} else {
		last_error = GetLastError();
	}

	return last_error;
}

