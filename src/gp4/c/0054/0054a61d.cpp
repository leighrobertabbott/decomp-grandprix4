#include <gp4/gp4.h>
#include <gp4/flags.h>

// After the flag-table clean-up 0x0054ad66, folds the 22-byte source table at 0x007a97b2 into the
// 22-byte per-car flag table at 0x007c2a48: every entry whose source byte has bit 7 gets 0xc0 set
// when its own bit 7 is clear or the switch byte [0x007aafdb] is non-zero. Then advances the 3-bit
// counter with 0x0054ba6b (eax low byte = new value, flags of its `and al, 7`) and `clc`s.
// Exit state: ebx = end of the flag table (from 0x0054ad66), ecx = 0, esi/edi one past the tables.
// FUNCTION: GP4 0x0054a61d
void __cdecl FUN_0054a61d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ad66, c);

    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x007a97b2);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint8_t al = 0;
    for (uint32_t left = 0x16u; left != 0u; --left) {
        al = *src++;
        if ((al & 0x80u) != 0u) {
            if ((*dst & 0x80u) == 0u || GP4_GLOBAL(uint8_t, 0x007aafdb) != 0u) {
                *dst |= 0xc0u;
            }
        }
        dst++;
    }

    c.eax = (c.eax & 0xffffff00u) | al;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = 0u;
    r->esi = 0x007a97b2u + 0x16u;
    r->edi = 0x007c2a48u + 0x16u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a61d, FUN_0054a61d, "regs(eax:u32) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043bcd4+src_0054a61d")
