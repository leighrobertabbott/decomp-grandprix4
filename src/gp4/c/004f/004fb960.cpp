#include <gp4/gp4.h>

// Byte-wise string compare (the inlined strcmp): 0 when equal, else -1 / 1 by the first differing byte.
static inline int CompareStrings(const uint8_t* a, const uint8_t* b) {
    for (;;) {
        uint8_t ca = *a;
        if (ca != *b) {
            return ca < *b ? -1 : 1;
        }
        if (ca == 0) {
            return 0;
        }
        a++;
        b++;
    }
}

// Linear search of a table of `count` C-string pointers for the string `key`: returns the index of the
// first entry equal to it, or -1.
// FUNCTION: GP4 0x004fb960
int __stdcall FUN_004fb960(const uint8_t** table, int32_t count, const uint8_t* key) {
    for (int32_t i = 0; i < count; i++) {
        if (CompareStrings(table[i], key) == 0) {
            return i;
        }
    }
    return -1;
}
GP4_IMPL(0x004fb960, FUN_004fb960, "stdcall(ptr[0x40]:{0*4/4:ptr[0x10]:u8{0x41,0x42,0,0}}, i32[-1..4], ptr[0x10]:u8{0x41,0x42,0,0}) -> i32")
