#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004582c0_Item {
    uint8_t Unknown00[0x14c];
    uint32_t Kind14c;
    uint8_t Unknown150[4];
    uint32_t Value154;
    uint8_t Unknown158[0x28];
    uint32_t Value180;
    uint8_t Unknown184[0x1cc - 0x184];
};
struct Unknown_004582c0 {
    uint8_t Unknown00[0x1ec];
    uint32_t Count1ec;
    Unknown_004582c0_Item* Items1f0;
};
#pragma pack(pop)

// Sums a per-item value over Count1ec items (0x1cc bytes each): Kind 2 -> Value154,
// Kind 1 -> Value154 / 3, anything else -> Value180.
// FUNCTION: GP4 0x004582c0
uint32_t __fastcall FUN_004582c0(Unknown_004582c0* self, void* /*edx*/) {
    uint32_t total = 0;
    uint32_t n = self->Count1ec;
    if (n != 0) {
        const Unknown_004582c0_Item* it = self->Items1f0;
        do {
            uint32_t v;
            if (it->Kind14c == 2) {
                v = it->Value154;
            } else if (it->Kind14c == 1) {
                v = it->Value154 / 3;
            } else {
                v = it->Value180;
            }
            total += v;
            it++;
            n--;
        } while (n != 0);
    }
    return total;
}
GP4_IMPL(0x004582c0, FUN_004582c0, "thiscall(ptr[0x200]:{0x1ec:u32[0..4], 0x1f0:ptr[0x740]:{*bytes, 0x14c*4/0x1cc:u32{0,1,2,3}}}) -> i32")
