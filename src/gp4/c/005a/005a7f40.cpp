#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a7f40 {
    uint8_t  Unknown00[4];
    uint16_t Count04;
    uint16_t Capacity06;
    uint32_t* Slots08;
    uint16_t* Values0C;
};
#pragma pack(pop)

// Clears every slot of the two parallel arrays and resets the used count.
// FUNCTION: GP4 0x005a7f40
int __fastcall FUN_005a7f40(Unknown_005a7f40* self, void* /*edx*/)
{
    uint16_t i = 0;
    if (self->Capacity06 != 0) {
        do {
            uint32_t idx = i;
            i = (uint16_t)(i + 1);
            self->Slots08[idx] = 0;
            self->Values0C[idx] = 0;
        } while (i < self->Capacity06);
    }
    self->Count04 = 0;
    return 0;
}
GP4_IMPL(0x005a7f40, FUN_005a7f40, "thiscall(ptr[0x20]:{4:u16[0..8], 6:u16[0..8], 8:ptr[0x40]:bytes, 0xc:ptr[0x40]:bytes}) -> i32")
