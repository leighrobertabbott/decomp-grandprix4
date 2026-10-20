#include <gp4/gp4.h>
#include <gp4/flags.h>

// Searches the string at esi for the byte al, stopping at the terminating zero.
// Found: eax = the address of the match, CF = 0 (other flags from the `cmp al, [esi]`, or from the
// `inc esi` that wrapped esi to 0). Not found (the zero byte reached first): eax unchanged, CF = 1,
// other flags from `cmp byte [esi], 0`. esi is preserved.
// FUNCTION: GP4 0x0054d924
void __cdecl FUN_0054d924(gp4::Regs* r) {
    const uint8_t target = static_cast<uint8_t>(r->eax);
    uint32_t pos = r->esi;

    uint32_t arith;
    for (;;) {
        const uint8_t ch = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(pos));
        if (target == ch) {
            r->eax = pos;
            arith = gp4::flags::sub8(target, ch) & ~gp4::flags::CF;
            break;
        }
        arith = gp4::flags::sub8(ch, 0u);
        if (ch == 0) {
            arith |= gp4::flags::CF;
            break;
        }
        arith = gp4::flags::inc32(pos, arith);
        ++pos;
        if (pos == 0) {
            r->eax = pos;
            arith &= ~gp4::flags::CF;
            break;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054d924, FUN_0054d924, "regs(eax:u32[0..4], esi:ptr[0x40]:u8{0,1,2,3}) -> (eax, cf, pf, af, zf, sf, of)")
