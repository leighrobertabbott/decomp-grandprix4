#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b43c0_Target { uint32_t* Vtable00; };
struct Unknown_005b43c0 { uint8_t Unknown00[4]; Unknown_005b43c0_Target* Target04; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b43c0_Slot40)(Unknown_005b43c0_Target* self, void* edx, int value);

// Forwards its stack argument to virtual slot 0x40 of the object held at +4 (a tail jump through
// the vtable, `jmp [eax+0x40]`); with no object there it returns 1.
// FUNCTION: GP4 0x005b43c0
int32_t __fastcall FUN_005b43c0(Unknown_005b43c0* self, void* /*edx*/, int32_t value) {
    Unknown_005b43c0_Target* target = self->Target04;
    if (target == 0) {
        return 1;
    }
    Unknown_005b43c0_Slot40 method = (Unknown_005b43c0_Slot40)target->Vtable00[0x40 / 4];
    return method(target, 0, value);
}
GP4_IMPL(0x005b43c0, FUN_005b43c0, "thiscall(ptr[0x10]:{4:nptr[0xc8]:{0:ptr[0x50]:{0x40:u32{=0x005bd340}}}}, i32) -> i32")
