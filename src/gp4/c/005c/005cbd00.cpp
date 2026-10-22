#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cbd00 { uint8_t Unknown00[0x24]; int32_t Value24; uint8_t Unknown28[0xd8]; };
#pragma pack(pop)

// Copies the dword at +0x24 of the object to *out (`mov eax,[ecx+0x24]; mov ecx,[esp+4]; mov [ecx],eax;
// ret 4`); eax keeps the copied value.
// FUNCTION: GP4 0x005cbd00
int32_t __fastcall FUN_005cbd00(const Unknown_005cbd00* self, void* /*edx*/, int32_t* out) {
    const int32_t value = self->Value24;
    *out = value;
    return value;
}
GP4_IMPL(0x005cbd00, FUN_005cbd00, "thiscall(ptr[0x100]:bytes, ptr[8]:bytes) -> i32")
