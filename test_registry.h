#ifndef NXDK_TESTS_TEST_REGISTRY_H
#define NXDK_TESTS_TEST_REGISTRY_H

#include <vector>

namespace TestRegistry {
struct TestCase {
  virtual ~TestCase() = default;
  virtual void operator()() = 0;
};

#define TEST_CASE(__method_name__)                                           \
  struct TEST##__method_name__ : TestRegistry::TestCase {                    \
    TEST##__method_name__() { TestRegistry::test_registry.push_back(this); } \
    void operator()() override { __method_name__(); }                        \
  };                                                                         \
  struct TEST##__method_name__ TEST##__method_name__##_instance__;

#define TEST_CASE(__method_name__, __setup_method_name__,                    \
                  __teardown_method_name__)                                  \
  struct TEST##__method_name__ : TestRegistry::TestCase {                    \
    TEST##__method_name__() { TestRegistry::test_registry.push_back(this); } \
    void operator()() override {                                             \
      if (__setup_method_name__()) {                                         \
        __method_name__();                                                   \
        __teardown_method_name__();                                          \
      }                                                                      \
    }                                                                        \
  };                                                                         \
  struct TEST##__method_name__ TEST##__method_name__##_instance__;

void run_tests();

extern std::vector<TestCase *> test_registry;

}  // namespace TestRegistry
#endif  // NXDK_TESTS_TEST_REGISTRY_H
