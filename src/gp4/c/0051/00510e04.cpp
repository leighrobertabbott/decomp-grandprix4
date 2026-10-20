#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests bit (cl & 7) of byte (cl >> 3) of the 32-byte "wanted" bitmap at 0x008245a0. cl is replaced
// by 0 when the bit is set and by its mask | 0x80 when it is clear; the flags are those of the final
// `and cl, 0` / `or cl, 0x80`. ebx is saved and restored; the upper bytes of ecx are kept.
// FUNCTION: GP4 0x00510e04
void __cdecl FUN_00510e04(gp4::Regs* r) {
    const uint32_t key = r->ecx & 0xffu;
    const uint32_t mask = 1u << (key & 7u);
    const uint32_t bits = GP4_ARRAY(uint8_t, 0x008245a0)[key >> 3];
    uint32_t cl = 0u;
    if ((bits & mask) == 0u) {
        cl = mask | 0x80u;
    }
    r->ecx = (r->ecx & 0xffffff00u) | cl;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cl));
}
GP4_IMPL(0x00510e04, FUN_00510e04, "regs(ecx:u32) -> (ecx, cf, pf, af, zf, sf, of); globals=layout=keys_008245a0")
