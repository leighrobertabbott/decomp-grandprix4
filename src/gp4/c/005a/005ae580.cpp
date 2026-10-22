#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ae580 { uint8_t Unknown00[0x274]; uint8_t Flag274; uint8_t Unknown275[0x20b]; };
#pragma pack(pop)

// Returns 0x7ffdd when the byte flag at +0x274 is nonzero, else 0
// (`mov al, [ecx+0x274]; neg al; sbb eax, eax; and eax, 0x7ffdd`).
// FUNCTION: GP4 0x005ae580
int32_t __fastcall FUN_005ae580(const Unknown_005ae580* self, void* /*edx*/) {
    return self->Flag274 != 0 ? 0x7ffdd : 0;
}
GP4_IMPL(0x005ae580, FUN_005ae580, "thiscall(ptr[0x280]:bytes) -> i32")
