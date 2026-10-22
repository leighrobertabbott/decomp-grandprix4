#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4400_Target { uint32_t* Vtable00; };
struct Unknown_005b4400 { uint8_t Unknown00[8]; Unknown_005b4400_Target* Target08; };
#pragma pack(pop)

typedef int (__fastcall *Unknown_005b4400_Slot34)(Unknown_005b4400_Target* self, void* edx, int value);

// Forwards its stack argument to virtual slot 0x34 of the object held at +8 (a tail jump through
// the vtable, `jmp [eax+0x34]`); with no object there it returns 1.
// FUNCTION: GP4 0x005b4400
int32_t __fastcall FUN_005b4400(Unknown_005b4400* self, void* /*edx*/, int32_t value) {
    Unknown_005b4400_Target* target = self->Target08;
    if (target == 0) {
        return 1;
    }
    Unknown_005b4400_Slot34 method = (Unknown_005b4400_Slot34)target->Vtable00[0x34 / 4];
    return method(target, 0, value);
}
GP4_IMPL(0x005b4400, FUN_005b4400, "thiscall(ptr[0x10]:{8:nptr[0xc8]:{0:ptr[0x50]:{0x34:u32{=0x005bd340}}}}, i32) -> i32")
