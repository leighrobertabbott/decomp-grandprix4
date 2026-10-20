#include <gp4/gp4.h>
#include <gp4/flags.h>

// Toggles the state byte [0x891529] on the "wanted" bitmap bit 0x4f (bit 7 of byte 9 of 0x008245a0,
// tested through 0x510e04 with ecx = 0x4f; ecx ends as that routine's cl: 0 when the bit is set,
// 0x80 when clear). Bit set: [0x891529] = 0xff. Bit clear and [0x891529] == 0xff: the byte is reset to 0,
// [0x6f3b7c] = [0x6f3b7d] = 0xff and [0x852d70] = 0x80. The flags are those of the last flag writer:
// the callee's `or cl, 0x80` / `and cl, 0` when the bit is set, else `cmp byte [0x891529], 0xff`.
// FUNCTION: GP4 0x00516c2b
void __cdecl FUN_00516c2b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = 0x4fu;
    gp4::call_regs(0x00510e04, c);

    uint32_t arith = c.eflags;
    if ((c.eflags & gp4::flags::ZF) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00891529) = 0xffu;
    } else {
        const uint8_t state = GP4_GLOBAL(uint8_t, 0x00891529);
        arith = gp4::flags::sub8(state, 0xffu);
        if (state == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x00891529) = 0u;
            GP4_GLOBAL(uint8_t, 0x006f3b7c) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x006f3b7d) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80u;
        }
    }
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516c2b, FUN_00516c2b, "regs() -> (ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0")
