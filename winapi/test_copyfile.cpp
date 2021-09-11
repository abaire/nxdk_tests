#include <windows.h>

#include <algorithm>

#include "test_registry.h"

using namespace TestRegistry;

static LPCSTR test_source_file = R"(Z:\test.dat)";
static uint8_t test_source_file_content[1024] = {0};

static BOOL CreateTestFile(LPCSTR filename, void *content,
                           DWORD content_length) {
  HANDLE h;

  h = CreateFile(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    PrintFailWithLastError("Failed to create test file.");
    return FALSE;
  }

  DWORD bytesWritten;
  BOOL success = WriteFile(h, content, content_length, &bytesWritten, nullptr);
  if (!success) {
    PrintFailWithLastError("Failed to write to test file.");
    CloseHandle(h);
    return FALSE;
  }

  return CloseHandle(h);
}

static BOOL VerifyFileContent(LPCSTR filename, void *expected_content,
                              DWORD expected_content_length) {
  HANDLE h = CreateFile(filename, FILE_GENERIC_READ, FILE_SHARE_READ, nullptr,
                        OPEN_ALWAYS, 0, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    PrintFailWithLastError("Failed to open target file '%s' for validation.",
                           filename);
    return FALSE;
  }

  uint8_t buffer[4096] = {0};
  const uint8_t *expected_content_start =
      static_cast<uint8_t *>(expected_content);
  const uint8_t *expected_read_ptr = static_cast<uint8_t *>(expected_content);

  while (TRUE) {
    DWORD bytes_read = 0;
    if (!ReadFile(h, buffer, sizeof(buffer), &bytes_read, nullptr)) {
      PrintFailWithLastError("Failed to read from '%s' for validation.",
                             filename);
      CloseHandle(h);
      return FALSE;
    }

    if (!bytes_read) {
      break;
    }

    DWORD offset = expected_read_ptr - expected_content_start;
    if (offset + bytes_read > expected_content_length) {
      PrintFail(
          "Validation of '%s' failed. File is larger than expected size %lu.",
          expected_content_length);
      CloseHandle(h);
      return FALSE;
    }

    auto end = expected_read_ptr + bytes_read;
    auto mismatch = std::mismatch(expected_read_ptr, end, buffer);
    if (mismatch.first < end) {
      PrintFail(
          "Validation of '%s' failed. Bytes at '%lu' differ: expected 0x%x but "
          "have 0x%x.",
          filename, offset + (mismatch.first - expected_read_ptr),
          *mismatch.first, *mismatch.second);
      CloseHandle(h);
      return FALSE;
    }

    expected_read_ptr += bytes_read;
  }

  CloseHandle(h);

  DWORD file_size = expected_read_ptr - expected_content_start;
  if (file_size < expected_content_length) {
    PrintFail(
        "Validation of '%s' failed. File size %lu is smaller than expected "
        "size %lu.",
        file_size, expected_content_length);
    return FALSE;
  }

  return TRUE;
}

static BOOL Setup() {
  for (int i = 0; i < 1024; ++i) {
    test_source_file_content[i] = (i & 0xFF);
  }

  return CreateTestFile(test_source_file, test_source_file_content,
                        sizeof(test_source_file_content));
}

static void Teardown() { DeleteFile(test_source_file); }

TEST_CASE(CopyFile_ToNewFile_Succeeds, Setup, Teardown) {
  BOOL success;
  static const LPCSTR target = R"(Z:\copied.dat)";

  success = CopyFile(test_source_file, target, FALSE);
  if (!success) {
    PrintFailWithLastError("Failed to copy test file to new path.");
    return FALSE;
  }

  success = VerifyFileContent(target, test_source_file_content,
                              sizeof(test_source_file_content));
  DeleteFile(target);

  return success;
}

TEST_CASE(CopyFile_ToEntirelyInvalidPath_Fails, Setup, Teardown) {
  BOOL success;
  static const LPCSTR target = R"(QQQ:\copied.dat)";

  success = CopyFile(test_source_file, target, FALSE);
  if (success) {
    PrintFailWithLastError("Copied test file to invalid path %s.", target);
    DeleteFile(target);
    return FALSE;
  }

  return TRUE;
}

TEST_CASE(CopyFile_ToNonExistentSubdir_Fails, Setup, Teardown) {
  BOOL success;
  static const LPCSTR target = R"(Z:\this\is\not__\a\__path__\copied.dat)";

  success = CopyFile(test_source_file, target, FALSE);
  if (success) {
    PrintFailWithLastError("Copied test file to non-existing path %s.", target);
    DeleteFile(target);
    return FALSE;
  }

  return TRUE;
}

TEST_CASE(CopyFile_OverExisting_Overwrite_Succeeds, Setup, Teardown) {
  static const LPCSTR target = R"(Z:\target.dat)";
  uint8_t buffer[1024] = {0};
  for (int i = 0; i < 1024; ++i) {
    buffer[i] = (1023 - i) & 0xFF;
  }

  if (!CreateTestFile(target, buffer, sizeof(buffer))) {
    PrintFailWithLastError("Failed to generate test file.");
    return FALSE;
  }

  BOOL success = CopyFile(test_source_file, target, FALSE);
  if (!success) {
    PrintFailWithLastError("Failed to overwrite target file.");
  }

  success = VerifyFileContent(target, test_source_file_content,
                              sizeof(test_source_file_content));

  DeleteFile(target);
  return success;
}

TEST_CASE(CopyFile_OverExisting_NoOverwrite_Fails, Setup, Teardown) {
  static const LPCSTR target = R"(Z:\target.dat)";
  uint8_t buffer[1024] = {0};
  for (int i = 0; i < 1024; ++i) {
    buffer[i] = (1023 - i) & 0xFF;
  }

  if (!CreateTestFile(target, buffer, sizeof(buffer))) {
    PrintFailWithLastError("Failed to generate test file.");
    return FALSE;
  }

  BOOL success = CopyFile(test_source_file, target, TRUE);
  if (success) {
    PrintFail("Overwrote target file when disallowed.");
    success = FALSE;
  } else {
    success = VerifyFileContent(target, buffer, sizeof(buffer));
  }

  DeleteFile(target);
  return success;
}

TEST_CASE(CopyFile_OverSelf_Fails, Setup, Teardown) {
  if (CopyFile(test_source_file, test_source_file, FALSE)) {
    PrintFailWithLastError("Was allowed to copy over self.");
    return FALSE;
  }

  if (GetLastError() != ERROR_SHARING_VIOLATION) {
    PrintFailWithLastError("Copy failed but did not flag sharing violation.");
    return FALSE;
  }

  return TRUE;
}

TEST_CASE(CopyFile_OverSelfWithNonexistentSource_Fails, Setup, Teardown) {
  if (CopyFile(R"(qq:\__fake_file.dat)", R"(qq:\__fake_file.dat)", FALSE)) {
    PrintFail("CopyFile signaled success copying non-existent file.");
    return FALSE;
  }

  if (GetLastError() != ERROR_INVALID_NAME) {
    PrintFailWithLastError("Copy failed but did not flag invalid source file as missing.");
    return FALSE;
  }

  return TRUE;
}

TEST_CASE(CopyFile_WithNonexistentSource_Fails, Setup, Teardown) {
  if (CopyFile(R"(qq:\__fake_file.dat)", R"(z:\nonexistentfile.dat)", FALSE)) {
    PrintFail("CopyFile signaled success copying non-existent file.");
    return FALSE;
  }

  if (GetLastError() != ERROR_INVALID_NAME) {
    PrintFailWithLastError("Copy failed but did not flag invalid source file as missing.");
    return FALSE;
  }

  return TRUE;
}
