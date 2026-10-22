#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00458270 {
    uint8_t Unknown00[0x1ec];
    uint32_t Count1ec;
    uint8_t* Records1f0;  // array of 0x1cc-byte records
};
#pragma pack(pop)

// Sums one value per 0x1cc-byte record of the Count1ec entries at +0x1f0: the record's kind dword
// (+0x14c) selects the dword at +0x158 (kind 2), that dword divided by 3 (kind 1) or the dword at +0x180.
// FUNCTION: GP4 0x00458270
uint32_t __fastcall FUN_00458270(Unknown_00458270* self, void* /*edx*/) {
    uint32_t total = 0;
    uint32_t remaining = self->Count1ec;
    if (remaining != 0) {
        const uint8_t* rec = self->Records1f0;
        do {
            uint32_t kind = *reinterpret_cast<const uint32_t*>(rec + 0x14c);
            uint32_t value;
            if (kind == 2) {
                value = *reinterpret_cast<const uint32_t*>(rec + 0x158);
            } else if (kind == 1) {
                value = *reinterpret_cast<const uint32_t*>(rec + 0x158) / 3;
            } else {
                value = *reinterpret_cast<const uint32_t*>(rec + 0x180);
            }
            total += value;
            rec += 0x1cc;
            remaining--;
        } while (remaining != 0);
    }
    return total;
}
GP4_IMPL(0x00458270, FUN_00458270, "thiscall(ptr[0x1f8]:{0x1ec:u32[0..4], 0x1f0:ptr[0x760]:{*bytes, 0x14c*4/0x1cc:u32{0,1,2,3}}}) -> u32")
