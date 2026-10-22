#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Slot_0048e020 {
    uint8_t Unknown00[0x168];
    int32_t Parent168;
    uint8_t Unknown16c[8];
    uint8_t Flag174;
    uint8_t Unknown175[0x2a0 - 0x175];
};
struct Unknown_0048e020 {
    Slot_0048e020 Slots[25];
};
#pragma pack(pop)

// Walks the parent chain (dword +0x168) starting at slot `index` (0x2a0-byte slots); index 0 ends the
// walk. Returns 0 when a slot on the way has its byte +0x174 clear, else 1.
// FUNCTION: GP4 0x0048e020
uint8_t __fastcall FUN_0048e020(Unknown_0048e020* self, void* /*edx*/, uint32_t index) {
    uint8_t result = 1;
    while (index != 0) {
        if (self->Slots[index].Flag174 == 0) {
            result = 0;
            break;
        }
        index = (uint32_t)self->Slots[index].Parent168;
    }
    return result;
}
GP4_IMPL(0x0048e020, FUN_0048e020, "thiscall(ptr[0x41a0]:layout=chain_0048e020, u32[0..24]) -> u8")
