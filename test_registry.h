#ifndef NXDK_TESTS_TEST_REGISTRY_H
#define NXDK_TESTS_TEST_REGISTRY_H

#include <vector>

namespace TestRegistry {
struct TestCase {
  virtual ~TestCase() = default;
  virtual void operator()() = 0;
};

#define TEST_CASE_BEGIN(__unique_name__)                                      \
  struct TEST##__unique_name__ : TestRegistry::TestCase {                     \
    TEST##__unique_name__() { TestRegistry::test_registry.push_back(this); } \
    void operator()() override {
#define TEST_CASE_END(__unique_name__) \
  }                                    \
  }                                    \
  ;                                    \
  struct TEST##__unique_name__ TEST##__unique_name__##_instance__;

void run_tests();

extern std::vector<TestCase *> test_registry;

}  // namespace TestRegistry
#endif  // NXDK_TESTS_TEST_REGISTRY_H
