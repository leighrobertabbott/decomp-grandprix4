#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005be2c0 { uint8_t Unknown00[0x138]; void* Ptr138; uint8_t Unknown13c[0x80]; float Value1bc; uint8_t Unknown1c0[0x40]; };
#pragma pack(pop)

// Returns (float from 0x005cc430 on the object at +0x138) * (float at +0x1bc) left unrounded on
// the x87 stack (fmul dword); ecx ends up holding the pointer at +0x138 (set before the call).
// FUNCTION: GP4 0x005be2c0
void __cdecl FUN_005be2c0(gp4::Regs* r) {
    Unknown_005be2c0* self = (Unknown_005be2c0*)r->ecx;
    void* inner = self->Ptr138;
    float first = gp4::thiscall_<0x005cc430, float>(inner);
    gp4::x87::Ext product = gp4::x87::ext_f32(first) * gp4::x87::ext_f32(self->Value1bc);
    gp4::x87::set_out_st(r, 0, product);
    r->ecx = (uint32_t)(uintptr_t)inner;
}
GP4_IMPL(0x005be2c0, FUN_005be2c0, "regs(ecx:ptr[0x200]:{0x138:ptr[0x21490]:{0x21480:f32}, 0x1bc:f32}) -> (st0, ecx)")
