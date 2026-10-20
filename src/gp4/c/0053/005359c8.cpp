#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies pieces of the saved-state buffer [0x007a93e0] into the block at 0x00822300: a word from
// 0x00535a93 (also kept in [0x007aafe8]), a word from 0x00535aa4, a byte from 0x00535ab7 and, unless
// the first word is 3 or 4, five bytes from 0x00535ae3 and four bytes from 0x00535ac8. The callees
// return the source addresses in eax. On exit eax is the last callee's result, esi/edi are one past
// the last source/destination byte, ecx is 0 after a `rep movsb` (else unchanged) and the flags are
// those of the last callee, or of the equal `cmp word [0x7aafe8], 3|4` on the early exit.
// FUNCTION: GP4 0x005359c8
void __cdecl FUN_005359c8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00822300);

    gp4::call_regs(0x00535a93, c);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));
    const uint16_t selector = *reinterpret_cast<const uint16_t*>(src);
    src += 2;
    *reinterpret_cast<uint16_t*>(dst) = selector;
    dst += 2;
    GP4_GLOBAL(uint16_t, 0x007aafe8) = selector;

    gp4::call_regs(0x00535aa4, c);
    src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));
    *reinterpret_cast<uint16_t*>(dst) = *reinterpret_cast<const uint16_t*>(src);
    src += 2;
    dst += 2;

    gp4::call_regs(0x00535ab7, c);
    src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));
    *dst++ = *src++;

    uint32_t ecx = r->ecx;
    uint32_t flags = gp4::flags::sub16(selector, 3u);
    if (selector != 3u) {
        flags = gp4::flags::sub16(selector, 4u);
        if (selector != 4u) {
            gp4::call_regs(0x00535ae3, c);
            src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));
            for (uint32_t i = 0u; i != 5u; ++i) {
                *dst++ = *src++;
            }

            gp4::call_regs(0x00535ac8, c);
            src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));
            for (uint32_t i = 0u; i != 4u; ++i) {
                *dst++ = *src++;
            }
            ecx = 0u;
            flags = c.eflags;
        }
    }

    r->eax = c.eax;
    r->ecx = ecx;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005359c8, FUN_005359c8, "regs(ecx:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_005359c8")
