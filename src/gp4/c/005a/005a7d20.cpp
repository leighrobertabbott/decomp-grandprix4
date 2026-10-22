#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a7d20 {
    uint8_t   Unknown00[4];
    uint16_t  Count04;
    uint16_t  Capacity06;
    uint32_t* Slots08;
    uint16_t* Values0C;
};
#pragma pack(pop)

// Clears slot `index` of the two parallel arrays and decrements the used count; 3 when the
// 16-bit index is out of range, 0 otherwise (also when the slot table is null).
// FUNCTION: GP4 0x005a7d20
int __fastcall FUN_005a7d20(Unknown_005a7d20* self, void* /*edx*/, uint32_t index)
{
    uint16_t i = (uint16_t)index;
    if (i >= self->Capacity06) {
        return 3;
    }
    if (self->Slots08 != 0) {
        self->Slots08[i] = 0;
        self->Values0C[i] = 0;
        self->Count04 = (uint16_t)(self->Count04 - 1);
    }
    return 0;
}
GP4_IMPL(0x005a7d20, FUN_005a7d20, "thiscall(ptr[0x20]:{4:u16, 6:u16[0..8], 8:nptr[0x40]:bytes, 0xc:ptr[0x40]:bytes}, u32{0,1,2,3,4,5,6,7,8,9,0x10001,0x20003}) -> i32")
