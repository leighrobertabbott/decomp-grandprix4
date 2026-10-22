#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004779e0 { void* Slots[0x10]; };
#pragma pack(pop)

// Calls FUN_00472dc0 (stores the dword at +0x484) on the object in slot `index`
// of this pointer table.
// FUNCTION: GP4 0x004779e0
void __fastcall FUN_004779e0(Unknown_004779e0* self, void* /*edx*/, int index, int value) {
    gp4::thiscall_<0x00472dc0, void>(self->Slots[index], value);
}
GP4_IMPL(0x004779e0, FUN_004779e0, "thiscall(ptr[0xc]:layout=objs_004779a0, i32[0..2], i32) -> void")
