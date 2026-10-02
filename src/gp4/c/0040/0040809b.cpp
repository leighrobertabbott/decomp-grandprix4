#include <gp4/gp4.h>

// the original is `mov eax, end; sub eax, start`: the compiler folds the two
// address immediates unless the subtraction hides behind a call boundary
static __declspec(noinline) int regionBytes(const uint8_t* end, const uint8_t* start) {
    return static_cast<int>(end - start);
}

// FUNCTION: GP4 0x0040809b
int __cdecl FUN_0040809b() {
    return regionBytes(GP4_ARRAY(uint8_t, 0x0063e18c), GP4_ARRAY(uint8_t, 0x00630224));
}
GP4_IMPL(0x0040809b, FUN_0040809b, "cdecl() -> i32")
