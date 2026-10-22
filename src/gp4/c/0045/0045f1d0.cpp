#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045f1d0 { uint8_t Unknown00[0xc]; uint32_t Key0c; uint32_t Key10; };
#pragma pack(pop)

// Ascending two-key comparator (qsort-style): unsigned +0xc first, then unsigned +0x10.
// FUNCTION: GP4 0x0045f1d0
int __cdecl FUN_0045f1d0(const Unknown_0045f1d0* a, const Unknown_0045f1d0* b) {
    if (a->Key0c < b->Key0c) return -1;
    if (b->Key0c < a->Key0c) return 1;
    if (a->Key10 < b->Key10) return -1;
    return b->Key10 < a->Key10 ? 1 : 0;
}
GP4_IMPL(0x0045f1d0, FUN_0045f1d0, "cdecl(ptr[0x18]:u32{=0,1,2,0x80000000,0xffffffff}, ptr[0x18]:u32{=0,1,2,0x80000000,0xffffffff}) -> i32")
