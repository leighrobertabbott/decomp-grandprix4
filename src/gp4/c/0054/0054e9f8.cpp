#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the selector byte [0x007aafd3] is 0xff and edx is 1, bit 7 of the byte at esi + 0x1f is cleared and
// edx becomes 0 (flags of that `and`); when it is 0xff and edx is 5, or any other value, edx is kept (flags of
// `cmp edx, 5`, resp. `cmp edx, 1`). When the selector byte is not 0xff, edx is cleared (flags of the
// selector compare).
// FUNCTION: GP4 0x0054e9f8
void __cdecl FUN_0054e9f8(gp4::Regs* r) {
    const uint8_t selector = GP4_GLOBAL(uint8_t, 0x007aafd3);
    uint32_t arith = gp4::flags::sub8(selector, 0xffu);
    if (selector == 0xffu) {
        const uint32_t edx = r->edx;
        arith = gp4::flags::sub32(edx, 1u);
        if (edx == 1u) {
            const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x1f) & 0x7fu);
            GP4_FIELD(uint8_t, r->esi, 0x1f) = cleared;
            arith = gp4::flags::logic8(cleared);
            r->edx = 0u;
        } else {
            arith = gp4::flags::sub32(edx, 5u);
        }
    } else {
        r->edx = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054e9f8, FUN_0054e9f8, "regs(edx:u32{0,1,1,5,5,2,0xffffffff}, esi:ptr[0x40]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
