#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb070 { uint8_t Unknown00[0x20]; int32_t Value20; uint8_t Unknown24[0xdc]; };
#pragma pack(pop)

// Copies the dword at +0x20 of the object to *out (`mov eax,[ecx+0x20]; mov ecx,[esp+4]; mov [ecx],eax;
// ret 4`); eax keeps the copied value.
// FUNCTION: GP4 0x005cb070
int32_t __fastcall FUN_005cb070(const Unknown_005cb070* self, void* /*edx*/, int32_t* out) {
    const int32_t value = self->Value20;
    *out = value;
    return value;
}
GP4_IMPL(0x005cb070, FUN_005cb070, "thiscall(ptr[0x100]:bytes, ptr[8]:bytes) -> i32")
