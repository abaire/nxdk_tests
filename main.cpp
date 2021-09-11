#include <hal/video.h>
#include <nxdk/mount.h>
#include <nxdk/path.h>
#include <windows.h>

#include <cassert>
#include <cstring>

#include "test_registry.h"

// Mounts the path containing this xbe as "A:".
static BOOL mount_drive_a() {
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

static BOOL mount_drives() {
  if (!mount_drive_a()) {
    DbgPrint("[ERROR]: Mounting error: Could not mount drive A\n");
    return FALSE;
  }
  if (!nxMountDrive('X', R"(\Device\Harddisk0\Partition3)")) {
    DbgPrint("[ERROR]: Mounting error: Could not mount drive X\n");
    return FALSE;
  }
  if (!nxMountDrive('Y', R"(\Device\Harddisk0\Partition4)")) {
    DbgPrint("[ERROR]: Mounting error: Could not mount drive Y\n");
    return FALSE;
  }
  if (!nxMountDrive('Z', R"(\Device\Harddisk0\Partition5)")) {
    DbgPrint("[ERROR]: Mounting error: Could not mount drive Z\n");
    return FALSE;
  }
  return TRUE;
}

int main() {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
  DbgPrint("Starting WinAPI tests");

  if (!mount_drives()) {
    return ERROR_GEN_FAILURE;
  }

  TestRegistry::run_tests();

  Sleep(60 * 1000);
  return 0;
}
