#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b4340 { uint8_t Unknown00[4]; const uint32_t* const* Object04; uint8_t Unknown08[0xf8]; };
#pragma pack(pop)

// If the object at +4 is set the routine returns 0; otherwise it dispatches through
// slot 0x30 of that (null) object's vtable with a null `this` and the same stack argument.
// FUNCTION: GP4 0x005b4340
int32_t __fastcall FUN_005b4340(Unknown_005b4340* self, void* /*edx*/, int32_t arg) {
    if (self->Object04 != nullptr) {
        return 0;
    }
    const uint32_t* vtbl = *self->Object04;
    typedef int32_t(__fastcall * Slot)(void*, void*, int32_t);
    return ((Slot)vtbl[0x30 / 4])(nullptr, nullptr, arg);
}
GP4_IMPL(0x005b4340, FUN_005b4340, "thiscall(ptr[0x100]:bytes, i32) -> i32")
