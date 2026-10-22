#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bb370_Inner { uint8_t Unknown00[0xc]; const int32_t* Pointer0c; uint8_t Unknown10[0x10]; };
struct Unknown_005bb370 { uint8_t Unknown00[4]; Unknown_005bb370_Inner* Inner04; uint8_t Unknown08[0xf8]; };
#pragma pack(pop)

// Follows the pointers at +4 then +0xc and returns the dword they designate; 0 if either is null.
// FUNCTION: GP4 0x005bb370
int32_t __fastcall FUN_005bb370(Unknown_005bb370* self, void* /*edx*/) {
    if (self->Inner04 != nullptr) {
        const int32_t* value = self->Inner04->Pointer0c;
        if (value != nullptr) {
            return *value;
        }
    }
    return 0;
}
GP4_IMPL(0x005bb370, FUN_005bb370, "thiscall(ptr[0x40]:{4:nptr[0x40]:{0xc:nptr[0x10]:bytes}}) -> i32")
