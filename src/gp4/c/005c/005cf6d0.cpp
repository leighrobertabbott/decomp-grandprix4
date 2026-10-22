#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cf6d0 {
    int32_t Value00;
    int32_t Value04;
    int32_t Value08;
    int32_t Value0c;
    int32_t Value10;
    int32_t Value14;
    int32_t Value18;
};
#pragma pack(pop)

// Stores its seven stack arguments into the first seven dwords of the object.
// FUNCTION: GP4 0x005cf6d0
void __fastcall FUN_005cf6d0(Unknown_005cf6d0* self, void* /*edx*/, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g) {
    self->Value00 = a;
    self->Value04 = b;
    self->Value08 = c;
    self->Value0c = d;
    self->Value10 = e;
    self->Value14 = f;
    self->Value18 = g;
}
GP4_IMPL(0x005cf6d0, FUN_005cf6d0, "thiscall(ptr[0x20]:bytes, i32, i32, i32, i32, i32, i32, i32) -> void")
