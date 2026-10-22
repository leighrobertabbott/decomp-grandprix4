#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004779a0 { void* Slots[0x10]; };
#pragma pack(pop)

// Calls FUN_00472d90 on the object in slot `index` of this pointer table, passing `value`.
// FUNCTION: GP4 0x004779a0
void __fastcall FUN_004779a0(Unknown_004779a0* self, void* /*edx*/, int index, int value) {
    gp4::thiscall_<0x00472d90, void>(self->Slots[index], value);
}
GP4_IMPL(0x004779a0, FUN_004779a0, "thiscall(ptr[0xc]:layout=objs_004779a0, i32[0..2], i32) -> void")
