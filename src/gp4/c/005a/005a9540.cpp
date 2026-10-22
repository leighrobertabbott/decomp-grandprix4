#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9540 { const uint32_t* Vptr00; uint8_t Unknown04[0x3c]; int32_t State40; uint8_t Unknown44[0xbc]; };
#pragma pack(pop)

// When the state at +0x40 is 2 the call is forwarded to vtable slot 0xe8 with the same object;
// otherwise returns 1.
// FUNCTION: GP4 0x005a9540
int32_t __fastcall FUN_005a9540(Unknown_005a9540* self, void* /*edx*/) {
    if (self->State40 == 2) {
        typedef int32_t(__fastcall * Slot)(Unknown_005a9540*, void*);
        return ((Slot)self->Vptr00[0xe8 / 4])(self, nullptr);
    }
    return 1;
}
GP4_IMPL(0x005a9540, FUN_005a9540, "thiscall(ptr[0x100]:{0:ptr[0x100]:{0xe8:u32{=0x005ccc10}}, 0x40:i32{2,0,1,3}}) -> i32")
