#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9260 {
    const uint32_t* Vptr00;
    uint8_t Unknown04[0x3c];
    uint32_t Value40;
    uint8_t Unknown44[8];
    uint32_t Value4c;
};
#pragma pack(pop)

// Validates two masks against virtual getters (vtable +0xdc and +0xe0): each argument must share a bit
// with the corresponding getter result, else 0x1f5 is returned. The first argument is stored at
// +0x4c as soon as it passes, the second at +0x40; returns 0 when both pass.
// FUNCTION: GP4 0x005a9260
int32_t __fastcall FUN_005a9260(Unknown_005a9260* self, void* /*edx*/, uint32_t first, uint32_t second) {
    typedef uint32_t(__fastcall * Getter)(Unknown_005a9260*, void*);
    uint32_t allowed = ((Getter)self->Vptr00[0xdc / 4])(self, nullptr);
    if ((first & allowed) == 0) {
        return 0x1f5;
    }
    self->Value4c = first;
    allowed = ((Getter)self->Vptr00[0xe0 / 4])(self, nullptr);
    if ((second & allowed) == 0) {
        return 0x1f5;
    }
    self->Value40 = second;
    return 0;
}
GP4_IMPL(0x005a9260, FUN_005a9260, "thiscall(ptr[0x60]:{0:ptr[0x100]:{0xdc:u32{=0x005b17a0}, 0xe0:u32{=0x005cb4c0}}, 4:u32{0,1,2,4,0xf}, 0x24:u32{0,1,2,4,0xf}}, u32{0,1,2,4,8,0xf}, u32{0,1,2,4,8,0xf}) -> i32")
