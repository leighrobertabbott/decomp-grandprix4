#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d42a0 { uint8_t Unknown00[0x238]; int32_t Value238; uint8_t Unknown23c[0x20]; };
#pragma pack(pop)

// Stores the argument at +0x238 of the object (`mov eax, [esp+4]; mov [ecx+0x238], eax; ret 4`);
// eax keeps the stored value.
// FUNCTION: GP4 0x005d42a0
int32_t __fastcall FUN_005d42a0(Unknown_005d42a0* self, void* /*edx*/, int32_t value) {
    self->Value238 = value;
    return value;
}
GP4_IMPL(0x005d42a0, FUN_005d42a0, "thiscall(ptr[0x260]:bytes, i32) -> i32")
