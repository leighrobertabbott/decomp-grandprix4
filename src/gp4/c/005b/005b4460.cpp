#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4460_Target { const uint32_t* Vptr00; uint8_t Unknown04[0x3c]; };
struct Unknown_005b4460 { uint8_t Unknown00[8]; Unknown_005b4460_Target* Target08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Returns 1 when there is no object at +8; otherwise forwards the stack argument to vtable
// slot 0x40 of that object (a tail jump through the vtable, `jmp [eax+0x40]`).
// FUNCTION: GP4 0x005b4460
int32_t __fastcall FUN_005b4460(Unknown_005b4460* self, void* /*edx*/, int32_t arg) {
    Unknown_005b4460_Target* target = self->Target08;
    if (target == nullptr) {
        return 1;
    }
    typedef int32_t(__fastcall * Slot)(Unknown_005b4460_Target*, void*, int32_t);
    return ((Slot)target->Vptr00[0x40 / 4])(target, nullptr, arg);
}
GP4_IMPL(0x005b4460, FUN_005b4460, "thiscall(ptr[0x40]:{8:nptr[0x40]:{0:ptr[0x100]:{0x40:u32{=0x005a9d50}}, 0x14:ptr[0x100]:bytes}}, i32[0..63]) -> i32")
