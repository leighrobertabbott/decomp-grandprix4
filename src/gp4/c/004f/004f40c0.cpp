#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004f40c0 {
    uint8_t Unknown00[0x14];
    int32_t* Counts14;
    int32_t* Indices18;
    uint8_t Unknown1c[0x634];
    uint8_t* Table650;
    uint8_t* Table654;
};
#pragma pack(pop)

// True when, for the record chosen through the index table at +0x18, the next Counts14[slot] consecutive
// records all have byte +0xc set in the first table (0x20-byte records at +0x650) and byte +0xc clear in
// the second table (0x18-byte records at +0x654); also true for a count of zero or less.
// FUNCTION: GP4 0x004f40c0
uint8_t __fastcall FUN_004f40c0(Unknown_004f40c0* self, void* /*edx*/, int32_t slot) {
    int32_t count = self->Counts14[slot];
    int32_t rec = self->Indices18[slot];
    uint8_t result = 1;
    if (count > 0) {
        const uint8_t* a = self->Table650 + rec * 0x20 + 0xc;
        int32_t offsetB = rec * 0x18;
        int32_t i = 0;
        for (;;) {
            if (*a == 0) {
                result = 0;
                break;
            }
            if (self->Table654[offsetB + 0xc] != 0) {
                result = 0;
                break;
            }
            i++;
            a += 0x20;
            offsetB += 0x18;
            if (i >= count) {
                break;
            }
        }
    }
    return result;
}
GP4_IMPL(0x004f40c0, FUN_004f40c0, "thiscall(ptr[0x660]:{0x14:ptr[0x40]:i32[-1..4], 0x18:ptr[0x40]:i32[0..3], 0x650:ptr[0x400]:u8{1,1,1,0}, 0x654:ptr[0x300]:u8{0,0,0,1}}, i32[0..15]) -> u8")
