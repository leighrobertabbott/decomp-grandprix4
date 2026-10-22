#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046f730 { uint8_t Unknown00[0xc84]; uint32_t Table0c84[18]; };
#pragma pack(pop)

// Stores the float 0x3fc90fdb (pi/2) in two float tables of the object: element `index` of the
// table at +0xc84 and element `index` of the table at +0xc8c.
// FUNCTION: GP4 0x0046f730
void __fastcall FUN_0046f730(Unknown_0046f730* self, void* /*edx*/, int index) {
    self->Table0c84[index] = 0x3fc90fdbu;
    self->Table0c84[index + 2] = 0x3fc90fdbu;
}
GP4_IMPL(0x0046f730, FUN_0046f730, "thiscall(ptr[0xd00]:bytes, i32[0..13]) -> void")
