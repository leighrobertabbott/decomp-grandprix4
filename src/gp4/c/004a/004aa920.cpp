#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004aa920 { uint8_t Unknown00[8]; float Key08; };
#pragma pack(pop)

// Descending three-way compare on the float at +8 (qsort-style comparator).
// FUNCTION: GP4 0x004aa920
int __cdecl FUN_004aa920(const Unknown_004aa920* a, const Unknown_004aa920* b) {
    if (b->Key08 < a->Key08) return -1;
    if (a->Key08 < b->Key08) return 1;
    return 0;
}
GP4_IMPL(0x004aa920, FUN_004aa920, "cdecl(ptr[0x10]:u32{=0x3f800000,0x40000000,0xbf800000,0x7fc00000,0x00000000}, ptr[0x10]:u32{=0x3f800000,0x40000000,0xbf800000,0x7fc00000,0x00000000}) -> i32")
