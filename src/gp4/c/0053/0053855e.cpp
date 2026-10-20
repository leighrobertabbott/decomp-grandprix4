#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053855e
// Bounded read from the current file (or memory image): nothing happens (flags of `cmp [0x829c68], 0`)
// when the mode byte [0x829c68] is 0. Otherwise 0x005385df opens the file (mode 2); the request offset esi
// is rebased by [0x829c5c] and the range esi .. esi + ecx must lie below the limit [0x829c60] without
// overflow. A valid request is read by 0x005386a5 (esi = rebased source, edi = destination, ecx = count) and
// the file is closed by 0x0053862f, CF clear. A range that overflows or exceeds the limit is closed first;
// an open failure or a rebase overflow skips the close; every failure then clears the mode byte
// (0x00538715) and sets CF.
void __cdecl FUN_0053855e(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829c68);
    if (mode == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005385df, c);
    uint32_t arith = c.eflags;
    bool valid = false;
    bool close = false;
    if ((arith & gp4::flags::CF) == 0u) {
        const uint32_t offset = c.esi;
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x00829c5c);
        c.esi = offset + base;
        arith = gp4::flags::add32(offset, base);
        if ((arith & gp4::flags::CF) == 0u) {
            c.eax = c.esi + c.ecx;
            arith = gp4::flags::add32(c.esi, c.ecx);
            if ((arith & gp4::flags::CF) != 0u) {
                close = true;
            } else {
                const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00829c60);
                arith = gp4::flags::sub32(c.eax, limit);
                if (c.eax > limit) {
                    close = true;
                } else {
                    valid = true;
                }
            }
        }
    }

    if (valid) {
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x005386a5, c);
        gp4::call_regs(0x0053862f, c);
        arith = c.eflags & ~gp4::flags::CF;
    } else {
        if (close) {
            c.eflags = gp4::flags::merge(c.eflags, arith);
            gp4::call_regs(0x0053862f, c);
            arith = c.eflags;
        }
        gp4::cdecl_<0x00538715, void>();
        arith |= gp4::flags::CF;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053855e, FUN_0053855e, "regs(esi:u32[0..0x100], edi:ptr[0x100]:bytes, ecx:u32[0..0x80]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tmpf_0053855e")
