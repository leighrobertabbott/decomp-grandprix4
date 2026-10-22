#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4360Inner { const uint32_t* Vptr00; uint8_t Unknown04[0x3c]; };
struct Unknown_005b4360 { uint8_t Unknown00[4]; Unknown_005b4360Inner* Object04; uint8_t Unknown08[0xf8]; };
#pragma pack(pop)

// Returns 1 when there is no object at +4; otherwise forwards the stack argument to vtable
// slot 0x34 of that object.
// FUNCTION: GP4 0x005b4360
int32_t __fastcall FUN_005b4360(Unknown_005b4360* self, void* /*edx*/, int32_t arg) {
    Unknown_005b4360Inner* obj = self->Object04;
    if (obj == nullptr) {
        return 1;
    }
    typedef int32_t(__fastcall * Slot)(Unknown_005b4360Inner*, void*, int32_t);
    return ((Slot)obj->Vptr00[0x34 / 4])(obj, nullptr, arg);
}
GP4_IMPL(0x005b4360, FUN_005b4360, "thiscall(ptr[0x40]:{4:nptr[0x40]:{0:ptr[0x100]:{0x34:u32{=0x005a9d50}}, 0x14:ptr[0x100]:bytes}}, i32[0..63]) -> i32")
