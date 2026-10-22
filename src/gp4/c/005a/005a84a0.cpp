#include <gp4/gp4.h>

struct Unknown_005a84a0;

typedef int32_t (__fastcall *Unknown_005a84a0_Slot80)(Unknown_005a84a0* self, void* edx, uint32_t index);

#pragma pack(push, 1)
struct Unknown_005a84a0 {
    uint32_t* Vtable00;
    uint8_t   Unknown04[0x32];
    uint16_t  Count36;
};
#pragma pack(pop)

// Asks the object (virtual slot 0x80, one call per index below Count36) which of its entries are busy:
// returns 2 when at least one answered non-zero, 1 when there are no entries at all, otherwise 0.
// FUNCTION: GP4 0x005a84a0
int32_t __fastcall FUN_005a84a0(Unknown_005a84a0* self, void* /*edx*/) {
    uint32_t index = 0;
    uint16_t busy = 0;
    if (self->Count36 != 0) {
        do {
            Unknown_005a84a0_Slot80 method = (Unknown_005a84a0_Slot80)self->Vtable00[0x80 / 4];
            if (method(self, 0, index) != 0) {
                busy = (uint16_t)(busy + 1);
            }
            index = index + 1;
        } while ((uint16_t)index < self->Count36);
        if (busy != 0) {
            return 2;
        }
    }
    return self->Count36 == busy;
}
GP4_IMPL(0x005a84a0, FUN_005a84a0, "thiscall(ptr[0x80]:{0:ptr[0x90]:{0x80:u32{=0x005cb0b0}}, 0x18:u32{=0,1,2,3,0x10}, 0x36:u16[0..4]}) -> i32")
