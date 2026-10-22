#include <gp4/gp4.h>

struct Unknown_005bb0f0 { uint32_t Value00; uint8_t Unknown04[0xfc]; };

// Constructor stub: clears the dword at +0 and returns this
// (`mov eax, ecx; mov dword [eax], 0; ret`).
// FUNCTION: GP4 0x005bb0f0
Unknown_005bb0f0* __fastcall FUN_005bb0f0(Unknown_005bb0f0* self, void* /*edx*/) {
    self->Value00 = 0;
    return self;
}
GP4_IMPL(0x005bb0f0, FUN_005bb0f0, "thiscall(ptr[0x100]:bytes) -> i32")
