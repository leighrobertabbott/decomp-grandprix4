#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d7ff0 {
    uint8_t Unknown00[0x708];
    uint32_t Value708; uint32_t Value70c;
    uint8_t Unknown710[8];
    uint32_t Value718; uint32_t Value71c;
    uint32_t Value720; uint32_t Value724;
    uint32_t Value728; uint32_t Value72c;
    uint8_t Unknown730[0x10];
};
#pragma pack(pop)

// Copies the dwords at +0x708, +0x718, +0x720, +0x728 to +0x70c, +0x71c, +0x724, +0x72c, then continues in
// 0x005cecc0 (`jmp`) on the same object.
// FUNCTION: GP4 0x005d7ff0
void __fastcall FUN_005d7ff0(Unknown_005d7ff0* self, void* /*edx*/) {
    self->Value72c = self->Value728;
    self->Value724 = self->Value720;
    self->Value71c = self->Value718;
    self->Value70c = self->Value708;
    gp4::thiscall_<0x005cecc0, void>(self);
}
GP4_IMPL(0x005d7ff0, FUN_005d7ff0, "thiscall(ptr[0x740]:bytes) -> void")
