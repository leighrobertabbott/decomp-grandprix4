#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests whether al is one of the six menu option bytes at 0x0081d60f or equals the nonzero byte at
// 0x0081d62e: CF = 1 when it is (`stc`), CF = 0 otherwise (`clc`). The other flags are those of the last
// compare: `cmp al, [ebx]` for a hit in the six bytes, `cmp byte [0x81d62e], 0` when that byte is zero,
// else `cmp al, [0x81d62e]`. ebx and ecx are saved and restored.
// FUNCTION: GP4 0x0052bd14
void __cdecl FUN_0052bd14(gp4::Regs* r) {
    const uint32_t al = r->eax & 0xffu;
    const uint8_t* options = GP4_ARRAY(const uint8_t, 0x0081d60f);
    uint32_t arith;
    uint32_t carry;

    uint32_t k = 0;
    while (k != 6u && al != options[k]) {
        ++k;
    }
    if (k != 6u) {
        arith = gp4::flags::sub8(al, options[k]);
        carry = 1u;
    } else {
        const uint32_t extra = GP4_GLOBAL(uint8_t, 0x0081d62e);
        if (extra == 0u) {
            arith = gp4::flags::sub8(extra, 0u);
            carry = 0u;
        } else {
            arith = gp4::flags::sub8(al, extra);
            carry = (al == extra) ? 1u : 0u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x0052bd14, FUN_0052bd14, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=set_0081d60f")
