#include "test_registry.h"

#include <string>
#include <vector>

namespace TestRegistry {

static std::vector<TestCase*> registry;

// Workaround for undefined static initialization ordering.
std::vector<TestCase*>& test_registry() { return registry; }

void run_tests() {
  std::vector<std::string> failed_tests;
  DWORD num_tests = 0;
  DWORD num_failures = 0;

  for (auto& it : test_registry()) {
    ++num_tests;
    if (!it->operator()()) {
      failed_tests.push_back(it->name());
      ++num_failures;
    }
  }

  DbgPrint("All tests completed - %d of %d failed.", num_failures, num_tests);
  for (const auto& it : failed_tests) {
    DbgPrint("[FAIL] %s", it.c_str());
  }
}

BOOL dummy_setup_() { return TRUE; }
void dummy_teardown_() {}

}  // namespace TestRegistry
