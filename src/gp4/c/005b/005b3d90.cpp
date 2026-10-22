#include <gp4/gp4.h>

struct Unknown_005b3d90;
typedef void (__fastcall *Unknown_005b3d90_SlotB4)(Unknown_005b3d90* self, void* edx);

#pragma pack(push, 1)
struct Unknown_005b3d90 { uint32_t* Vtable00; uint8_t Unknown04[0x191]; uint8_t Flag195; };
#pragma pack(pop)

// Clears the byte at +0x195, calls virtual slot 0xb4 on the object (no arguments) and returns 0.
// FUNCTION: GP4 0x005b3d90
int32_t __fastcall FUN_005b3d90(Unknown_005b3d90* self, void* /*edx*/) {
    Unknown_005b3d90_SlotB4 method = (Unknown_005b3d90_SlotB4)self->Vtable00[0xb4 / 4];
    self->Flag195 = 0;
    method(self, 0);
    return 0;
}
GP4_IMPL(0x005b3d90, FUN_005b3d90, "thiscall(ptr[0x1a0]:{0:ptr[0xc0]:{0xb4:u32{=0x005b3d80}}}) -> i32")
