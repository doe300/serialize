
#include "cpptest-main.h"

extern void registerSimpleTests();
extern void registerBytePackingTests();
extern void registerBitPackingTests();
extern void registerTypeSafeTests();

int main(int argc, char** argv) {
  registerSimpleTests();
  registerBytePackingTests();
  registerBitPackingTests();
  registerTypeSafeTests();
  return Test::runSuites(argc, argv);
}
