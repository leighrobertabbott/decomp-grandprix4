#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004f3f90 {
    uint8_t Unknown00[0x18];
    int32_t* Indices18;
    uint8_t Unknown1c[0x634];
    uint8_t* Table650;
    uint8_t* Table654;
};
#pragma pack(pop)

// True when, for the record chosen through the index table at +0x18, the first table (0x20-byte
// records at +0x650) has bytes +0x1ec and +0x20c set and the second table (0x18-byte records at
// +0x654) has bytes +0x174 and +0x18c clear.
// FUNCTION: GP4 0x004f3f90
uint8_t __fastcall FUN_004f3f90(Unknown_004f3f90* self, void* /*edx*/, int32_t slot) {
    int32_t rec = self->Indices18[slot];
    uint8_t result = 0;
    uint8_t* a = self->Table650 + rec * 0x20;
    if (a[0x1ec] != 0) {
        uint8_t* b = self->Table654 + rec * 0x18;
        if (b[0x174] == 0 && a[0x20c] != 0 && b[0x18c] == 0) {
            result = 1;
        }
    }
    return result;
}
GP4_IMPL(0x004f3f90, FUN_004f3f90, "thiscall(ptr[0x660]:{0x18:ptr[0x40]:i32[0..3], 0x650:ptr[0x400]:u8{0,1}, 0x654:ptr[0x400]:u8{0,1}}, i32[0..15]) -> u8")
