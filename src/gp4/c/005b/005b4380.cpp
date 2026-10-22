#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4380_Target { uint32_t* Vtable00; };
struct Unknown_005b4380 { uint8_t Unknown00[4]; Unknown_005b4380_Target* Target04; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b4380_Slot38)(Unknown_005b4380_Target* self, void* edx);

// Forwards to virtual slot 0x38 of the object held at +4 (a tail jump through the vtable,
// `jmp [eax+0x38]`); with no object there it returns 1.
// FUNCTION: GP4 0x005b4380
int32_t __fastcall FUN_005b4380(Unknown_005b4380* self, void* /*edx*/) {
    Unknown_005b4380_Target* target = self->Target04;
    if (target == 0) {
        return 1;
    }
    Unknown_005b4380_Slot38 method = (Unknown_005b4380_Slot38)target->Vtable00[0x38 / 4];
    return method(target, 0);
}
GP4_IMPL(0x005b4380, FUN_005b4380, "thiscall(ptr[0x10]:{4:nptr[0xc8]:{0:ptr[0x80]:{0x38:u32{=0x005bc220}}}}) -> i32")
