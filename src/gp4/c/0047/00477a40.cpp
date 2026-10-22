#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00477a40 { void* Slots[0x10]; };
#pragma pack(pop)

// Calls FUN_00472db0 on the object in slot `index` of this pointer table, passing `value`. The callee
// returns its byte result in al only; eax keeps the upper bytes of `value`, which this wrapper loaded into
// eax before the call.
// FUNCTION: GP4 0x00477a40
int __fastcall FUN_00477a40(Unknown_00477a40* self, void* /*edx*/, int index, int value) {
    uint8_t result = gp4::thiscall_<0x00472db0, uint8_t>(self->Slots[index], value);
    return (int)(((uint32_t)value & 0xffffff00u) | result);
}
GP4_IMPL(0x00477a40, FUN_00477a40, "thiscall(ptr[0xc]:layout=objs_004779a0, i32[0..2], i32) -> i32")
