#ifndef NXDK_TESTS_TEST_REGISTRY_H
#define NXDK_TESTS_TEST_REGISTRY_H

#include <windows.h>

#include <string>
#include <vector>

namespace TestRegistry {

struct TestCase {
  virtual ~TestCase() = default;
  virtual std::string name() const = 0;
  virtual BOOL operator()() = 0;

  BOOL verbose = TRUE;
};

inline void PrintFail(LPCSTR message) { DbgPrint("[ERROR]: %s", message); }

template <typename... VarArgs>
inline void PrintFail(LPCSTR message, VarArgs &&...args) {
  int string_length = snprintf(nullptr, 0, message, args...);
  std::string buf;
  buf.resize(string_length);

  snprintf(&buf[0], string_length, message, args...);
  PrintFail(buf.c_str());
}

inline void PrintFailWithLastError(LPCSTR message) {
  DbgPrint("[ERROR]: %s 0x%x", message, GetLastError());
}

template <typename... VarArgs>
inline void PrintFailWithLastError(LPCSTR message, VarArgs &&...args) {
  int string_length = snprintf(nullptr, 0, message, args...);
  std::string buf;
  buf.resize(string_length);

  snprintf(&buf[0], string_length, message, args...);
  PrintFailWithLastError(buf.c_str());
}

#define TEST_CASE_BASIC(__method_name__) \
  TEST_CASE(__method_name__, TestRegistry::dummy_setup_, TestRegistry::dummy_teardown_)

#define TEST_CASE(__method_name__, __setup_method_name__,                    \
                  __teardown_method_name__)                                  \
  static BOOL __method_name__();                                             \
  struct TEST##__method_name__ : TestRegistry::TestCase {                    \
    TEST##__method_name__() { TestRegistry::test_registry().push_back(this); } \
    std::string name() const override { return #__method_name__; }           \
    BOOL operator()() override {                                             \
      if (verbose) {                                                         \
        DbgPrint("<Testing> " #__method_name__);                               \
      }                                                                      \
      if (!__setup_method_name__()) {                                        \
        return FALSE;                                                        \
      }                                                                      \
      BOOL ret = __method_name__();                                          \
      __teardown_method_name__();                                            \
      if (verbose) {                                                         \
        DbgPrint(#__method_name__ " [%s]", ret ? "PASS" : "FAIL");           \
      }                                                                      \
      return ret;                                                            \
    }                                                                        \
  };                                                                         \
  struct TEST##__method_name__ TEST##__method_name__##_instance__;           \
  static BOOL __method_name__()

void run_tests();

BOOL dummy_setup_();
void dummy_teardown_();

std::vector<TestCase *>& test_registry();

}  // namespace TestRegistry
#endif  // NXDK_TESTS_TEST_REGISTRY_H
