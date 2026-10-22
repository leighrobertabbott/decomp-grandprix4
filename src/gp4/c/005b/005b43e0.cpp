#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b43e0_Target { uint32_t* Vtable00; };
struct Unknown_005b43e0 { uint8_t Unknown00[8]; Unknown_005b43e0_Target* Target08; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b43e0_Slot30)(Unknown_005b43e0_Target* self, void* edx, int value);

// Forwards its stack argument to virtual slot 0x30 of the object held at +8 (a tail jump through
// the vtable, `jmp [eax+0x30]`); with no object there it returns 0.
// FUNCTION: GP4 0x005b43e0
int32_t __fastcall FUN_005b43e0(Unknown_005b43e0* self, void* /*edx*/, int32_t value) {
    Unknown_005b43e0_Target* target = self->Target08;
    if (target == 0) {
        return 0;
    }
    Unknown_005b43e0_Slot30 method = (Unknown_005b43e0_Slot30)target->Vtable00[0x30 / 4];
    return method(target, 0, value);
}
GP4_IMPL(0x005b43e0, FUN_005b43e0, "thiscall(ptr[0x10]:{8:nptr[0xc8]:{0:ptr[0x40]:{0x30:u32{=0x005bd340}}}}, i32) -> i32")
