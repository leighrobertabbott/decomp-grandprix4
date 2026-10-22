#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_0045fb20 {
    uint8_t Unknown00[0x60];
    float Value60;
    uint32_t Unknown64;
};
#pragma pack(pop)

// Stores `value` (+0x60) and `extra` (+0x64) in the object, publishes value * [0x005f0660] to the
// float global 0x009fb4a4, and sets the dword global 0x009fb4a8 to the __ftol (0x005daabc) result of
// value * [0x005f065c], clamped to 3 when it is above 3 as an unsigned number.
// FUNCTION: GP4 0x0045fb20
void __fastcall FUN_0045fb20(Unknown_0045fb20* self, void* /*edx*/, float value, uint32_t extra) {
    using namespace gp4::x87;
    GP4_GLOBAL(float, 0x009fb4a4) = value * GP4_GLOBAL(float, 0x005f0660);
    self->Value60 = value;
    self->Unknown64 = extra;
    const Ext v = ext_f32(self->Value60) * ext_f32(GP4_GLOBAL(float, 0x005f065c));
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, v);
    gp4::call_regs(0x005daabc, c);
    GP4_GLOBAL(uint32_t, 0x009fb4a8) = 3;
    if (c.eax < 4) {
        GP4_GLOBAL(uint32_t, 0x009fb4a8) = c.eax;
    }
}
GP4_IMPL(0x0045fb20, FUN_0045fb20, "thiscall(ptr[0x70]:bytes, f32, i32) -> void")
