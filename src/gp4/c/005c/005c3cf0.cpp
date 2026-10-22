#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c3cf0 { uint8_t Unknown00[0x18]; int32_t Value18; uint8_t Unknown1c[0xe4]; };
#pragma pack(pop)

// Stores the argument at +0x18 of the object (`mov eax, [esp+4]; mov [ecx+0x18], eax; ret 4`);
// eax keeps the stored value.
// FUNCTION: GP4 0x005c3cf0
int32_t __fastcall FUN_005c3cf0(Unknown_005c3cf0* self, void* /*edx*/, int32_t value) {
    self->Value18 = value;
    return value;
}
GP4_IMPL(0x005c3cf0, FUN_005c3cf0, "thiscall(ptr[0x100]:bytes, i32) -> i32")
