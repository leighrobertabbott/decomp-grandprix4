#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of dword `index` of the table at 0x00833ff0.
static __declspec(noinline) uint32_t TableSlot(uint32_t index) {
    return (index << 2) + 0x00833ff0u;
}

// When [0x0081e5c4] >= 0x40, walks the 33 table dwords at 0x00833ff0 (the 0x54cb71 no-op runs before
// each read) and calls the empty routine 0x0054e232 (esi = 0x0054e270) for each one that differs
// from [0x00833f58]; the pushal/popal pairs restore every register. Then eax = 1 after a final
// 0x54cb71 call. The flags are those of `cmp eax, 0x20` (counter 0x21) after the loop, else of the
// initial `cmp dword [0x81e5c4], 0x40`.
// FUNCTION: GP4 0x0054e247
void __cdecl FUN_0054e247(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0081e5c4);
    uint32_t flags = gp4::flags::sub32(count, 0x40u);
    if (count >= 0x40u) {
        uint32_t index = 0u;
        do {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = index;
            gp4::call_regs(0x0054cb71, c);
            if (GP4_GLOBAL(uint32_t, TableSlot(index)) != GP4_GLOBAL(uint32_t, 0x00833f58)) {
                gp4::Regs d = *r;
                d.st_in = 0;
                d.st_out = 0;
                d.esi = 0x0054e270u;
                gp4::call_regs(0x0054e232, d);
            }
            ++index;
            flags = gp4::flags::sub32(index, 0x20u);
        } while (index <= 0x20u);
    }

    gp4::Regs e = *r;
    e.st_in = 0;
    e.st_out = 0;
    e.eax = 1u;
    gp4::call_regs(0x0054cb71, e);

    r->eax = 1u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e247, FUN_0054e247, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_0054e247")
