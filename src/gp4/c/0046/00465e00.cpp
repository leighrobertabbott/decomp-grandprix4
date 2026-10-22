#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00465e00_Inner {
    uint8_t Unknown00[0x23c];
    uint8_t Flag23c;
};
struct Unknown_00465e00 {
    uint8_t Unknown00[4];
    Unknown_00465e00_Inner* Inner4;
};
#pragma pack(pop)

// Translates a bit mask of input states into a flag word: bit0 -> 0x2, bit1 -> 0x10 (only while
// the byte at +0x23c of the object at +4 is zero), bit2 -> 0x40, bit3 -> 0x80, and the highest set
// bit among 0x80/0x40/0x20/0x10 -> 0x400/0x300/0x200/0x100 in bits 8..10.
// FUNCTION: GP4 0x00465e00
uint32_t __fastcall FUN_00465e00(Unknown_00465e00* self, void* /*edx*/, uint8_t mask) {
    uint32_t result = 0;
    if ((mask & 1) != 0) {
        result = 2;
    }
    if ((mask & 2) != 0 && self->Inner4->Flag23c == 0) {
        result |= 0x10;
    }
    if ((mask & 4) != 0) {
        result |= 0x40;
    }
    if ((mask & 8) != 0) {
        result |= 0x80;
    }
    if ((int8_t)mask < 0) {
        return result | 0x400;
    }
    if ((mask & 0x40) != 0) {
        return result | 0x300;
    }
    if ((mask & 0x20) != 0) {
        return result | 0x200;
    }
    if ((mask & 0x10) != 0) {
        result |= 0x100;
    }
    return result;
}
GP4_IMPL(0x00465e00, FUN_00465e00, "thiscall(ptr[0x10]:{4:ptr[0x244]:{0x23c:u8{0,1}}}, u8) -> u32")
