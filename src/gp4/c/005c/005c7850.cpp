#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c7850 {
    uint8_t Unknown00[0xe0];
    int32_t Value0e0;
    int32_t Value0e4;
    int32_t Value0e8;
    int32_t Value0ec;
    uint8_t Unknown0f0[0x1bc];
    int32_t Value2ac;
    int32_t Value2b0;
    uint8_t Unknown2b4[0x10];
    int32_t Value2c4;
    int32_t Value2c8;
};
#pragma pack(pop)

// Copies four dwords from the block at +0x2ac..+0x2c8 into +0xe0..+0xec and returns the address of the
// destination block (+0xe0): the original forms it with `lea eax, [ecx+0xe0]` and callers use eax.
// FUNCTION: GP4 0x005c7850
uint32_t __fastcall FUN_005c7850(Unknown_005c7850* self, void* /*edx*/) {
    int32_t* destination = &self->Value0e0;
    destination[0] = self->Value2b0;
    destination[1] = self->Value2ac;
    destination[2] = self->Value2c8;
    destination[3] = self->Value2c4;
    return reinterpret_cast<uint32_t>(destination);
}
GP4_IMPL(0x005c7850, FUN_005c7850, "thiscall(ptr[0x300]:bytes) -> u32")
