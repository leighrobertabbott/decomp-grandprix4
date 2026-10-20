#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills the five-word window of the table at 0x9e2310 starting at index [0x96a3b4]: the first
// [0x66d88d] words (a byte) get the word read from the table 0x9e22c2[index], the rest of the five
// get the word 0x9e231a[index]. Nothing is written when either table word is 0xffff. pushal/popal
// keep every register; the flags are those of the last compare (cmp ax,-1 / cmp dx,-1, or the final
// `cmp ecx, 5` of the fill loops).

// FUNCTION: GP4 0x0051efa6
void __cdecl FUN_0051efa6(gp4::Regs* r) {
    const int32_t limit = static_cast<int32_t>(GP4_GLOBAL(uint8_t, 0x0066d88d));
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0096a3b4);
    const uint16_t first = GP4_ARRAY(uint16_t, 0x009e22c2)[base];
    uint32_t flags = gp4::flags::sub16(first, 0xffffu);
    if (first != 0xffffu) {
        const uint16_t second = GP4_ARRAY(uint16_t, 0x009e231a)[base];
        flags = gp4::flags::sub16(second, 0xffffu);
        if (second != 0xffffu) {
            int32_t n = 0;
            for (; n < limit; ++n) {
                GP4_ARRAY(uint16_t, 0x009e2310)[static_cast<uint32_t>(n) + base] = first;
            }
            for (; n < 5; ++n) {
                GP4_ARRAY(uint16_t, 0x009e2310)[static_cast<uint32_t>(n) + base] = second;
            }
            flags = gp4::flags::sub32(static_cast<uint32_t>(n), 5u);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051efa6, FUN_0051efa6, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=idx_0051efa6")
