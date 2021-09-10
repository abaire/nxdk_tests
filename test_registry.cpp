#include "test_registry.h"

namespace TestRegistry {
std::vector<TestCase*> test_registry;

void run_tests() {
  for (auto& it : test_registry) {
    it->operator()();
  }
}

}  // namespace TestRegistry
