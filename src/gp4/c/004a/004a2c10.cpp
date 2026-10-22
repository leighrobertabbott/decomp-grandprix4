#include <gp4/gp4.h>

// Three-way comparison (qsort style) of two entries of the float table at 0x00a565b0, selected by the
// ints the two arguments point to: -1 when the first is smaller, 1 when the second is smaller,
// else 0 (equal or unordered).
// FUNCTION: GP4 0x004a2c10
int __cdecl FUN_004a2c10(const int* a, const int* b) {
    const float* table = GP4_ARRAY(float, 0x00a565b0);
    int ia = *a;
    int ib = *b;
    if (table[ia] < table[ib]) {
        return -1;
    }
    if (table[ib] < table[ia]) {
        return 1;
    }
    return 0;
}
GP4_IMPL(0x004a2c10, FUN_004a2c10, "cdecl(ptr[4]:i32[0..7], ptr[4]:i32[0..7]) -> i32; globals=fuzz:i32")
