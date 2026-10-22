#include <gp4/gp4.h>

struct Unknown_005a9d90;
typedef int (__fastcall *Unknown_005a9d90_Slot54)(Unknown_005a9d90* self, void* edx, uint32_t value);

#pragma pack(push, 1)
struct Unknown_005a9d90 {
    uint32_t* Vtable00;
    uint8_t   Unknown04[0x4c];
    uint16_t  Count50;
    uint8_t   Unknown52[2];
    uint16_t* Values54;
};
#pragma pack(pop)

// Calls virtual slot 0x54 once for every 16-bit entry of the array at +0x54, then empties it
// (count at +0x50 reset to 0) and returns 0.
// FUNCTION: GP4 0x005a9d90
int __fastcall FUN_005a9d90(Unknown_005a9d90* self, void* /*edx*/)
{
    uint16_t i = 0;
    if (self->Count50 != 0) {
        do {
            Unknown_005a9d90_Slot54 method = (Unknown_005a9d90_Slot54)self->Vtable00[0x54 / 4];
            method(self, 0, self->Values54[i]);
            i = (uint16_t)(i + 1);
        } while (i < self->Count50);
    }
    self->Count50 = 0;
    return 0;
}
GP4_IMPL(0x005a9d90, FUN_005a9d90, "thiscall(ptr[0x60]:{0:ptr[0x58]:{0x54:u32{=0x005a8380}}, 0x50:u16[0..6], 0x54:ptr[0x20]:bytes}) -> i32")
