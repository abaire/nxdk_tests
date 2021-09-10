#include <cassert>
#include <cstring>
#include <hal/video.h>
#include <nxdk/mount.h>
#include <nxdk/path.h>
#include <windows.h>

// Mounts the path containing this xbe as "A:".
static BOOL MountDiskA() {
  if (nxIsDriveMounted('A')) {
    DbgPrint("A: already mounted!");
    return FALSE;
  }

  CHAR targetPath[MAX_PATH];
  nxGetCurrentXbeNtPath(targetPath);

  LPSTR filenameStr;
  filenameStr = strrchr(targetPath, '\\');
  assert(filenameStr != NULL);
  *(filenameStr + 1) = '\0';

  BOOL status = nxMountDrive('A', targetPath);
  return status;
}

static void TestCopyFile() {
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

}

void RunTests() {
  TestCopyFile();
}

int main() {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
  DbgPrint("Starting WinAPI tests");

  MountDiskA();
  if (!nxMountDrive('X', R"(\Device\Harddisk0\Partition3)")) {
    DbgPrint("Mounting error: Could not mount drive X\n");
  }
  if (!nxMountDrive('Y', R"(\Device\Harddisk0\Partition4)")) {
    DbgPrint("Mounting error: Could not mount drive Y\n");
  }
  if (!nxMountDrive('Z', R"(\Device\Harddisk0\Partition5)")) {
    DbgPrint("Mounting error: Could not mount drive Z\n");
  }

  RunTests();

  return 0;
}
