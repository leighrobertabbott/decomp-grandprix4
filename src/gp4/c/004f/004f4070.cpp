#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Slot_004f4070 {
    uint8_t Unknown00[0xc];
    uint8_t Flag0c;
    uint8_t Unknown0d[0x13];
    uint8_t Flag20;
};
struct Entry20_004f4070 {
    uint8_t Unknown00[0xc];
    uint8_t Flag0c;
    uint8_t Unknown0d[0x1f];
};
struct Entry18_004f4070 {
    uint8_t Unknown00[0xc];
    uint8_t Flag0c;
    uint8_t Unknown0d[0xb];
};
struct Unknown_004f4070 {
    uint8_t Unknown00[0x18];
    int32_t* Indices18;
    uint8_t Unknown1c[0x634];
    uint8_t* Table650;
    uint8_t* Table654;
};
#pragma pack(pop)

// True when, for the record chosen through the index table at +0x18, the first table (0x20-byte records at
// +0x650) has bytes +0xc and +0x2c set and the second table (0x18-byte records at +0x654) has bytes +0xc and
// +0x24 clear.
// FUNCTION: GP4 0x004f4070
uint8_t __fastcall FUN_004f4070(Unknown_004f4070* self, void* /*edx*/, int32_t slot) {
    int32_t rec = self->Indices18[slot];
    uint8_t result = 0;
    uint8_t* a = self->Table650 + rec * 0x20;
    if (a[0xc] != 0) {
        uint8_t* b = self->Table654 + rec * 0x18;
        if (b[0xc] == 0 && a[0x2c] != 0 && b[0x24] == 0) {
            result = 1;
        }
    }
    return result;
}
GP4_IMPL(0x004f4070, FUN_004f4070, "thiscall(ptr[0x660]:{0x18:ptr[0x40]:i32[0..3], 0x650:ptr[0x100]:u8{0,0,1}, 0x654:ptr[0x100]:u8{0,0,1}}, i32[0..15]) -> u8")
