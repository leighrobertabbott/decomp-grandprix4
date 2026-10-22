#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb140 { uint8_t Unknown00[0x18]; int32_t Value18; uint8_t Unknown1c[0xe4]; };
#pragma pack(pop)

// Copies the dword at +0x18 of the object to *out (`mov eax,[ecx+0x18]; mov ecx,[esp+4]; mov [ecx],eax;
// ret 4`); eax keeps the copied value.
// FUNCTION: GP4 0x005cb140
int32_t __fastcall FUN_005cb140(const Unknown_005cb140* self, void* /*edx*/, int32_t* out) {
    const int32_t value = self->Value18;
    *out = value;
    return value;
}
GP4_IMPL(0x005cb140, FUN_005cb140, "thiscall(ptr[0x100]:bytes, ptr[8]:bytes) -> i32")
