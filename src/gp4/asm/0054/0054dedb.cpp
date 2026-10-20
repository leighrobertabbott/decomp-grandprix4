#include <gp4/gp4.h>
#include <gp4/flags.h>

// Removes every newline (0x0a) from the NUL-terminated text that the edit record at edi starts at
// ([edi] = text start, [edi+4] = end pointer, [edi+8] = cursor): each newline is deleted by the gap
// closer 0x0054e13f (eax = 1, edx = its address, so the following bytes move down and the end pointer
// drops), and the cursor [edi+8] drops by one when the newline lay before it. The scan restarts on the
// byte that moved into the gap. It ends with esi one past the terminating NUL, al = 0 (eax = 0 when a
// newline was removed, else the caller's eax with al cleared), edx = the last newline's address (or the
// caller's) and the flags of `or al, al` on the NUL (ZF and PF only).
// FUNCTION: GP4 0x0054dedb
void __cdecl FUN_0054dedb(gp4::Regs* r) {
    uint32_t* record = (uint32_t*)(uintptr_t)r->edi;
    uint32_t cursorSource = record[0];
    uint32_t lastNewline = r->edx;
    bool removed = false;
    for (;;) {
        const uint32_t ch = *(const uint8_t*)(uintptr_t)cursorSource;
        cursorSource += 1u;
        if (ch == 0u) {
            break;
        }
        if (ch != 0x0au) {
            continue;
        }
        cursorSource -= 1u;
        removed = true;
        lastNewline = cursorSource;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eax = 1u;
        c.edx = cursorSource;
        c.esi = cursorSource;
        gp4::call_regs(0x0054e13f, c);
        if (cursorSource < record[2]) {
            record[2] -= 1u;
        }
    }
    r->esi = cursorSource;
    r->edx = lastNewline;
    r->eax = removed ? 0u : (r->eax & 0xffffff00u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
}
GP4_IMPL(0x0054dedb, FUN_0054dedb, "regs(edi:ptr[0x10]:layout=txt_0054dedb, eax:u32, edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=txt_0054dedb")
