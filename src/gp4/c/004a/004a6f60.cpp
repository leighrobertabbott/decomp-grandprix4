#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004a6f60 { uint8_t Unknown00[4]; float Key04; };
#pragma pack(pop)

// Descending comparator on the float at +4 of two records: -1 when a's value is greater than b's,
// 1 when b's is greater than a's, otherwise 0 (equal or unordered).
// FUNCTION: GP4 0x004a6f60
int __cdecl FUN_004a6f60(const Unknown_004a6f60* a, const Unknown_004a6f60* b) {
    if (a->Key04 > b->Key04) {
        return -1;
    }
    if (b->Key04 > a->Key04) {
        return 1;
    }
    return 0;
}
GP4_IMPL(0x004a6f60, FUN_004a6f60, "cdecl(ptr[12]:u8{0x00,0x7f,0xc0}, ptr[12]:u8{0x00,0x7f,0xc0}) -> i32")
