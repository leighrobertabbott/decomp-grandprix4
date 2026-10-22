#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0050a0a0 {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint16_t Unknown10;
};
#pragma pack(pop)

// Clears the 0x12-byte record (four dwords and a word) and returns the record pointer.
// FUNCTION: GP4 0x0050a0a0
uint32_t __fastcall FUN_0050a0a0(Unknown_0050a0a0* self, void* /*edx*/) {
    self->Unknown00 = 0;
    self->Unknown04 = 0;
    self->Unknown08 = 0;
    self->Unknown0c = 0;
    self->Unknown10 = 0;
    return (uint32_t)(uintptr_t)self;
}
GP4_IMPL(0x0050a0a0, FUN_0050a0a0, "thiscall(ptr[0x20]:bytes) -> u32")
