#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sums the per-place counts of a driver pair: edx indexes a pair of bytes at 0x007c2a48 + 2*edx; for each
// of the two bytes, row = (byte & 0x3f) - 1; a row below 0x16 adds the count 0x00548836 returns for it
// (bl = ebx is its key; it keeps ebx/ecx/edx/esi) to eax, a larger row adds nothing. ebx, ecx, edx and esi
// are saved and restored, so the outputs are eax and the flags of the last flag writer: `cmp edx, 0x16`
// (row skipped) or the closing `add eax, edx` (count + sum so far) of the second byte.
// FUNCTION: GP4 0x0054885d
void __cdecl FUN_0054885d(gp4::Regs* r) {
    const uint32_t key = r->ebx;
    uint32_t cursor = 0x007c2a48u + (r->edx + r->edx);
    uint32_t total = 0u;
    uint32_t flags = 0u;
    for (uint32_t pass = 0u; pass < 2u; ++pass) {
        const uint32_t row = (static_cast<uint32_t>(GP4_FIELD(uint8_t, cursor, 0)) & 0x3fu) - 1u;
        ++cursor;
        flags = gp4::flags::sub32(row, 0x16u);
        if (row < 0x16u) {
            gp4::Regs c = {};
            c.ebx = key;
            c.edx = row;
            gp4::call_regs(0x00548836, c);
            flags = gp4::flags::add32(c.eax, total);
            total += c.eax;
        }
    }
    r->eax = total;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054885d, FUN_0054885d, "regs(edx:u32[0..10], ebx:u32{1,2,3,0x16}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=cnt_00548836+slots_005401a2")
