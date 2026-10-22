#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d3940 {
    uint32_t Vtable00;
    uint8_t Unknown04[4];
    uint32_t Words08[0x15];
    uint16_t Marks5c[0x15];
    uint16_t Marks86[0x15];
    uint8_t Blocks0b0[0x15][0x20];
};
#pragma pack(pop)

// Constructor: stores the vtable pointer 0x005f0efc, then for each of the 0x15 entries clears a dword
// (+8 array) and a 0x20-byte block (+0xb0 array) and sets one word in each of two word arrays
// (+0x5c and +0x86) to 0xffff. Returns the object.
// FUNCTION: GP4 0x004d3940
Unknown_004d3940* __fastcall FUN_004d3940(Unknown_004d3940* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0efc;
    for (int i = 0; i < 0x15; i++) {
        for (int j = 0; j < 0x20; j++) {
            self->Blocks0b0[i][j] = 0;
        }
        self->Words08[i] = 0;
        self->Marks5c[i] = 0xffff;
        self->Marks86[i] = 0xffff;
    }
    return self;
}
GP4_IMPL(0x004d3940, FUN_004d3940, "thiscall(ptr[0x360]:bytes) -> u32")
