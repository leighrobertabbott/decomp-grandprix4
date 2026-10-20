#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053859e
// Bounded write to the current file (or memory image), twin of 0x0053855e with the rebased operand in edi:
// nothing happens (flags of `cmp [0x829c68], 0`) when the mode byte [0x829c68] is 0. Otherwise 0x005385df
// opens the file (mode 2); the request offset edi is rebased by [0x829c5c] and the range edi .. edi + ecx
// must lie below the limit [0x829c60] without overflow. A valid request is written by 0x00538635 (esi =
// source, edi = rebased destination, ecx = count); success closes the file (0x0053862f) with CF clear, a
// failed write closes it and then clears the mode byte (0x00538715) with CF set. An open failure, a rebase
// or range overflow, or a range beyond the limit clears the mode byte without closing, CF set.
void __cdecl FUN_0053859e(gp4::Regs* r) {
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
    if ((arith & gp4::flags::CF) == 0u) {
        const uint32_t offset = c.edi;
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x00829c5c);
        c.edi = offset + base;
        arith = gp4::flags::add32(offset, base);
        if ((arith & gp4::flags::CF) == 0u) {
            c.eax = c.edi + c.ecx;
            arith = gp4::flags::add32(c.edi, c.ecx);
            if ((arith & gp4::flags::CF) == 0u) {
                const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00829c60);
                arith = gp4::flags::sub32(c.eax, limit);
                valid = (c.eax <= limit);
            }
        }
    }

    if (valid) {
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00538635, c);
        arith = c.eflags;
        const bool failed = (arith & gp4::flags::CF) != 0u;
        gp4::call_regs(0x0053862f, c);
        arith = c.eflags;
        if (!failed) {
            arith &= ~gp4::flags::CF;
        } else {
            gp4::cdecl_<0x00538715, void>();
            arith |= gp4::flags::CF;
        }
    } else {
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
GP4_IMPL(0x0053859e, FUN_0053859e, "regs(esi:ptr[0x100]:bytes, edi:u32[0..0x100], ecx:u32[0..0x80]) -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tmpf_0053855e")
