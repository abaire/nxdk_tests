#include <windows.h>

#include "test_registry.h"



TEST_CASE_BEGIN(TestCopyFile)

    WIN32_FIND_DATA find_data;
    HANDLE h;
    uint8_t buffer[1024] = {0};

    h = CreateFile(R"(Z:\test.dat)", GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        DbgPrint("Failed to create test file. 0x%x", GetLastError());
        return;
    }

    for (int i = 0; i < 1024; ++i) {
        buffer[i] = (i & 0xFF);
    }

    DWORD bytesWritten;
    BOOL success = WriteFile(h, buffer, sizeof(buffer), &bytesWritten, nullptr);
    if (!success) {
        DbgPrint("Failed to write to test file. 0x%x", GetLastError());
        CloseHandle(h);
        return;
    }

    CloseHandle(h);

    h = FindFirstFile(R"(Z:\*)", &find_data);
    if (h != INVALID_HANDLE_VALUE) {

        do {
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                DbgPrint("Dir: %s\n", find_data.cFileName);
            } else {
                LARGE_INTEGER filesize;
                filesize.LowPart = find_data.nFileSizeLow;
                filesize.HighPart = find_data.nFileSizeHigh;

                DbgPrint("File: %s - %lld\n", find_data.cFileName, filesize.QuadPart);
            }
        } while (FindNextFile(h, &find_data));

        CloseHandle(h);
    } else {
        DbgPrint("[ERROR] Failed to find files in Z:\\. 0x%x", GetLastError());
    }

    success = CopyFile(R"(Z:\test.dat)", R"(Z:\copied.dat)", FALSE);
    if (!success) {
        DbgPrint("[ERROR] Failed to copy test file to new path. 0x%x", GetLastError());
    }

TEST_CASE_END(TestCopyFile)