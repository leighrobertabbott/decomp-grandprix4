#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b6be0 { uint8_t Unknown00[0x26a0]; void* Flags26a0; };
#pragma pack(pop)

// Passes its stack argument to 0x005c2460 on the sub-object pointed to by the field at +0x26a0.
// The original loads the argument into eax before the call and 0x005c2460 only replaces al, so
// the full eax it returns (argument with the low byte replaced) is what callers read.
// FUNCTION: GP4 0x005b6be0
uint32_t __fastcall FUN_005b6be0(Unknown_005b6be0* self, void* /*edx*/, int32_t value) {
    gp4::Regs c = {};
    c.eax = (uint32_t)value;
    c.ecx = (uint32_t)(uintptr_t)self->Flags26a0;
    const uint32_t args[1] = {(uint32_t)value};
    gp4::call_regs_stdcall(0x005c2460, c, args);
    return c.eax;
}
GP4_IMPL(0x005b6be0, FUN_005b6be0, "thiscall(ptr[0x26a8]:{0x26a0:ptr[0x10]:bytes}, i32) -> u32")
