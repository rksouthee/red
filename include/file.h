#ifndef RED_FILE_H
#define RED_FILE_H

#include <Windows.h>
#include <string>
#include "buffer.h"

DWORD file_open(std::string filename, Buffer& buffer);
DWORD file_save(Buffer& buffer);

#endif
