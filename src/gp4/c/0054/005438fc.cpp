#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line so the window base 0x00830b00 is the only global the field reads see (the original indexes
// it through esi).
static __declspec(noinline) const uint32_t* windowAt(uint32_t address) {
    return reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
}

// After the record-table search 0x00543917 (CF clear = a selected record at index eax), checks that index
// against the window { start = [0x00830b00], length = [0x00830b08] }: an index below the start or at least
// `length` past it clears the selection bits of all 40 records (0x005438a0). esi ends as 0x00830b00 when the
// window was consulted; eax = index - start then; the flags are those of the last compare.
// FUNCTION: GP4 0x005438fc
void __cdecl FUN_005438fc(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00543917, c);
    if ((c.eflags & gp4::CF) == 0u) {
        const uint32_t* window = windowAt(0x00830b00u);
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(window));
        const uint32_t start = window[0];
        const uint32_t index = c.eax;
        c.eax = index - start;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(index, start));
        bool clear = index < start;
        if (!clear) {
            const uint32_t length = window[2];
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, length));
            clear = c.eax >= length;
        }
        if (clear) {
            gp4::call_regs(0x005438a0, c);
        }
    }
    *r = c;
}
GP4_IMPL(0x005438fc, FUN_005438fc, "regs(eax:u32, cf:u32[0..1], pf:u32[0..1], af:u32[0..1], zf:u32[0..1], sf:u32[0..1], of:u32[0..1]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sort_00542224+recs_005438a0+cur_00543942")
