#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b42b0_Target { uint32_t* Vtable00; };
struct Unknown_005b42b0 { uint8_t Unknown00[4]; Unknown_005b42b0_Target* Target04; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b42b0_Slot7c)(Unknown_005b42b0_Target* self, void* edx);

// Calls virtual slot 0x7c of the object held at +4 (a tail jump through the vtable,
// `jmp [eax+0x7c]`) and returns its result; with no object there it returns 4.
// FUNCTION: GP4 0x005b42b0
int32_t __fastcall FUN_005b42b0(Unknown_005b42b0* self, void* /*edx*/) {
    Unknown_005b42b0_Target* target = self->Target04;
    if (target == 0) {
        return 4;
    }
    Unknown_005b42b0_Slot7c method = (Unknown_005b42b0_Slot7c)target->Vtable00[0x7c / 4];
    return method(target, 0);
}
GP4_IMPL(0x005b42b0, FUN_005b42b0, "thiscall(ptr[0x10]:{4:nptr[0x1a0]:{0:ptr[0x80]:{0x7c:u32{=0x005b3d80}}}}) -> i32")
