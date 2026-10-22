#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d8930 { uint8_t Unknown00[0x274]; };
#pragma pack(pop)

// Calls the empty routine 0x005ae990, then tail-jumps to FUN_004d77a0 (the sound manager's shutdown
// of its sound-system object) with the same object.
// FUNCTION: GP4 0x004d8930
void __fastcall FUN_004d8930(Unknown_004d8930* self, void* /*edx*/) {
    gp4::cdecl_<0x005ae990, void>();
    gp4::thiscall_<0x004d77a0, void>(self);
}
GP4_IMPL(0x004d8930, FUN_004d8930, "thiscall(ptr[0x274]:layout=gpaudio_004d77a0) -> void")
