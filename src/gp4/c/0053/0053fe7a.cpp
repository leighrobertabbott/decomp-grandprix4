#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the slot of the id in eax up with 0x00541d06 (a miss returns 0 at once with ZF/PF from the
// `or eax, eax`); a hit gives the slot index (0x00541d44, eax = (address - 0x00b1b8d8) / 0x1c) and the
// 22 key bytes at 0x007c2a1c are scanned for the index's low byte (`repne scasb`): eax = -1 when it is
// there, else 0. The flags are those of the last byte compare (all equal on a hit). ecx and edi are
// saved and restored; every other register comes back as the two callees leave it.
// FUNCTION: GP4 0x0053fe7a
void __cdecl FUN_0053fe7a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00541d06, c);
    uint32_t arith = gp4::flags::logic32(c.eax);
    uint32_t result = 0u;
    if (c.eax != 0u) {
        gp4::call_regs(0x00541d44, c);
        const uint32_t key = c.eax & 0xffu;
        const uint8_t* const keys = GP4_ARRAY(const uint8_t, 0x007c2a1c);
        for (uint32_t i = 0; i < 0x16u; ++i) {
            const uint32_t value = keys[i];
            arith = gp4::flags::sub8(key, value);
            if (key == value) {
                result = 0xffffffffu;
                break;
            }
        }
    }

    r->eax = result;
    r->ebx = c.ebx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053fe7a, FUN_0053fe7a, "regs(eax:u32{0,1,0x1f1f,0xffffffff,5}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+keys_007c2a1c")
