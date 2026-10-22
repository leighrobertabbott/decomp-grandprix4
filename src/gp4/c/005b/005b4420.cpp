#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4420_Target { uint32_t* Vtable00; };
struct Unknown_005b4420 { uint8_t Unknown00[8]; Unknown_005b4420_Target* Target08; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b4420_Slot38)(Unknown_005b4420_Target* self, void* edx);

// Forwards to virtual slot 0x38 of the object held at +8 (a tail jump through the vtable,
// `jmp [eax+0x38]`); with no object there it returns 1.
// FUNCTION: GP4 0x005b4420
int32_t __fastcall FUN_005b4420(Unknown_005b4420* self, void* /*edx*/) {
    Unknown_005b4420_Target* target = self->Target08;
    if (target == 0) {
        return 1;
    }
    Unknown_005b4420_Slot38 method = (Unknown_005b4420_Slot38)target->Vtable00[0x38 / 4];
    return method(target, 0);
}
GP4_IMPL(0x005b4420, FUN_005b4420, "thiscall(ptr[0x10]:{8:nptr[0xc8]:{0:ptr[0x80]:{0x38:u32{=0x005bc220}}}}) -> i32")
