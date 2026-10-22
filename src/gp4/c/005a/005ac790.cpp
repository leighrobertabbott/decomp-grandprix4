#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ac790 { uint8_t Unknown00[4]; uint16_t Value04; uint8_t Unknown06[0xfa]; };
#pragma pack(pop)

// Copies the word at +4 of the object to *out and returns 0
// (`mov ax, [ecx+4]; mov ecx, [esp+4]; mov [ecx], ax; xor eax, eax; ret 4`).
// FUNCTION: GP4 0x005ac790
int32_t __fastcall FUN_005ac790(const Unknown_005ac790* self, void* /*edx*/, uint16_t* out) {
    *out = self->Value04;
    return 0;
}
GP4_IMPL(0x005ac790, FUN_005ac790, "thiscall(ptr[0x100]:bytes, ptr[8]:bytes) -> i32")
