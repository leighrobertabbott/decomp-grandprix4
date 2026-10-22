#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_00474d40 {
    uint8_t Flag00;
    uint8_t Unknown01[0x131];
    uint32_t Value132;
    uint32_t Value136;
    uint32_t Value13a;
    uint32_t Value13e;
    uint8_t Unknown142[0x12];
};
struct Unknown_00474d40 {
    uint8_t Unknown00[0x468];
    int32_t Last468;
    uint8_t Unknown46c[0x49a - 0x46c];
    Entry_00474d40 Entries49a[4];
};
#pragma pack(pop)

// Resets entries 0..Last468 (0x154 bytes each, starting at +0x49a): sets the flag byte to 1,
// clears the dwords +0x132 and +0x136 and copies +0x13e to +0x13a.
// FUNCTION: GP4 0x00474d40
void __fastcall FUN_00474d40(Unknown_00474d40* self, void* /*edx*/) {
    for (int i = 0; i < self->Last468 + 1; i++) {
        Entry_00474d40* entry = &self->Entries49a[i];
        entry->Value132 = 0;
        entry->Flag00 = 1;
        entry->Value13a = entry->Value13e;
        entry->Value136 = 0;
    }
}
GP4_IMPL(0x00474d40, FUN_00474d40, "thiscall(ptr[0xa00]:u32{=0xffffffff,0xfffffffe,0,1,2,3,0x80808080,0xa5a5a5a5}) -> void")
