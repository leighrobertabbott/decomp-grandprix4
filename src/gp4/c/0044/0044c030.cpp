#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_0044c030_Item {
    uint8_t Unknown00[0x10];
    float Key10;
    uint8_t Unknown14[0x58 - 0x14];
};
#pragma pack(pop)

// Comparator over indices into the 0x58-byte record array at [0x9fafac]: orders by the float at
// +0x10 descending (-1 when the first key is greater, 1 when it is smaller, else 0).
// FUNCTION: GP4 0x0044c030
int __cdecl FUN_0044c030(const int* a, const int* b) {
    Unknown_0044c030_Item* items = GP4_GLOBAL(Unknown_0044c030_Item*, 0x009fafac);
    Unknown_0044c030_Item* ia = &items[*a];
    Unknown_0044c030_Item* ib = &items[*b];
    if (ia->Key10 > ib->Key10) {
        return -1;
    }
    if (ia->Key10 < ib->Key10) {
        return 1;
    }
    return 0;
}
GP4_IMPL(0x0044c030, FUN_0044c030, "cdecl(ptr[4]:i32[0..3], ptr[4]:i32[0..3]) -> i32; gfields={0x009fafac:ptr[0x160]:{0x10*4/0x58:u32{=0xbfc00000,0,0x3f800000,0x40200000,0x7fc00000}}}")
