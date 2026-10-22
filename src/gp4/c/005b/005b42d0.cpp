#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b42d0_Target { uint32_t* Vtable00; };
struct Unknown_005b42d0 { uint8_t Unknown00[4]; Unknown_005b42d0_Target* Target04; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b42d0_Slot74)(Unknown_005b42d0_Target* self, void* edx);

// Forwards to virtual slot 0x74 of the object held at +4 (a tail jump through the vtable,
// `jmp [eax+0x74]`); with no object there it returns 4.
// FUNCTION: GP4 0x005b42d0
int32_t __fastcall FUN_005b42d0(Unknown_005b42d0* self, void* /*edx*/) {
    Unknown_005b42d0_Target* target = self->Target04;
    if (target == 0) {
        return 4;
    }
    Unknown_005b42d0_Slot74 method = (Unknown_005b42d0_Slot74)target->Vtable00[0x74 / 4];
    return method(target, 0);
}
GP4_IMPL(0x005b42d0, FUN_005b42d0, "thiscall(ptr[0x10]:{4:nptr[0xc8]:{0:ptr[0x80]:{0x74:u32{=0x005bc220}}}}) -> i32")
