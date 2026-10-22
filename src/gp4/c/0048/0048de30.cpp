#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Slot_0048de30 {
    uint8_t Unknown00[0x168];
    int32_t Link168;
    uint8_t Unknown16c[8];
    uint8_t Flag174;
    uint8_t Unknown175[0x2a0 - 0x175];
};
struct Unknown_0048de30 {
    Slot_0048de30 Slots[25];
};
#pragma pack(pop)

// Builds a 25-bit mask over the 25 slots (0x2a0 bytes each): bit i is set when slot i is active
// (byte +0x174 nonzero) and following its link chain (dword +0x168 names the next slot, repeating the
// test for each) ends in a link of 0 or 0x19. An inactive slot anywhere on the chain contributes no bit.
// FUNCTION: GP4 0x0048de30
uint32_t __fastcall FUN_0048de30(Unknown_0048de30* self, void* /*edx*/) {
    uint32_t mask = 0;
    uint32_t bit = 1;
    for (int i = 0; i < 0x19; i++) {
        int idx = i;
        for (;;) {
            if (self->Slots[idx].Flag174 == 0) {
                break;
            }
            int link = self->Slots[idx].Link168;
            if (link == 0 || link == 0x19) {
                mask |= bit;
                break;
            }
            idx = link;
        }
        bit += bit;
    }
    return mask;
}
GP4_IMPL(0x0048de30, FUN_0048de30, "thiscall(ptr[0x4200]:{0x174*25/0x2a0:u8{=0,1,1}, 0x168:u32{=0,25}, 0x408:u32{=0,25}, 0x6a8:u32{=0,25,1}, 0x948:u32{=0,25,2,1}, 0xbe8:u32{=0,25,3,1}, 0xe88:u32{=0,25,4,1}, 0x1128:u32{=0,25,5,1}, 0x13c8:u32{=0,25,6,1}, 0x1668:u32{=0,25,7,1}, 0x1908:u32{=0,25,8,1}, 0x1ba8:u32{=0,25,9,1}, 0x1e48:u32{=0,25,10,1}, 0x20e8:u32{=0,25,11,1}, 0x2388:u32{=0,25,12,1}, 0x2628:u32{=0,25,13,1}, 0x28c8:u32{=0,25,14,1}, 0x2b68:u32{=0,25,15,1}, 0x2e08:u32{=0,25,16,1}, 0x30a8:u32{=0,25,17,1}, 0x3348:u32{=0,25,18,1}, 0x35e8:u32{=0,25,19,1}, 0x3888:u32{=0,25,20,1}, 0x3b28:u32{=0,25,21,1}, 0x3dc8:u32{=0,25,22,1}, 0x4068:u32{=0,25,23,1}}) -> u32")
