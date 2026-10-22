#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004596f0_Item {
    uint8_t Unknown00[0x130];
    uint32_t Value130;
    uint8_t Unknown134[0x1cc - 0x134];
};
struct Unknown_004596f0 {
    uint8_t Unknown00[0x1ec];
    uint32_t Count1ec;
    Unknown_004596f0_Item* Items1f0;
};
#pragma pack(pop)

// Scans the Count1ec items (0x1cc bytes each) and returns false as soon as an item has a zero
// dword at +0x130, true when every item scanned has a nonzero value.
// FUNCTION: GP4 0x004596f0
bool __fastcall FUN_004596f0(Unknown_004596f0* self, void* /*edx*/) {
    uint32_t i = 0;
    bool ok = true;
    while (i < self->Count1ec) {
        if (self->Items1f0[i].Value130 == 0) {
            ok = false;
        }
        i++;
        if (!ok) {
            break;
        }
    }
    return ok;
}
GP4_IMPL(0x004596f0, FUN_004596f0, "thiscall(ptr[0x1f8]:layout=rec_004596b0) -> u8")
