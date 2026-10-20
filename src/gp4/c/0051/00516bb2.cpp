#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looked up through a helper so the compiler keeps the table base 0x891bd0 (the original reads
// [0x20*4 + 0x891bd0]).
static __declspec(noinline) uint32_t TableEntry(const uint32_t* table, uint32_t index) {
    return table[index];
}

// When the byte flag [0x7a93b7] is set, temporarily forces the state bytes 0x88fd96/97/9f/a0 and
// 0x88fd94/95 (al = 0x27 stored through 0x51d48e, al = 0x38 through 0x51d494, 0x88fd96 = 4,
// 0x88fd97 = 0, 0x88fd94 = 1, 0x88fd95 = 0), calls 0x51693f with esi = [0x891bd0 + 0x20*4], then
// restores 0x88fd96/97 and 0x88fd9f/a0 (0x88fd94/95 stay changed). The flags are those of
// `cmp byte [0x7a93b7], 0` on both paths; when active ax ends as the saved word at 0x88fd96 and
// esi as the table entry.
// FUNCTION: GP4 0x00516bb2
void __cdecl FUN_00516bb2(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007a93b7);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
    if (enabled == 0u) {
        return;
    }

    const uint8_t saved96 = GP4_GLOBAL(uint8_t, 0x0088fd96);
    const uint8_t saved97 = GP4_GLOBAL(uint8_t, 0x0088fd97);
    const uint8_t saved9f = GP4_GLOBAL(uint8_t, 0x0088fd9f);
    const uint8_t savedA0 = GP4_GLOBAL(uint8_t, 0x0088fda0);

    gp4::Regs c = {};
    c.eax = 0x27u;
    gp4::call_regs(0x0051d48e, c);
    c.eax = 0x38u;
    gp4::call_regs(0x0051d494, c);

    GP4_GLOBAL(uint8_t, 0x0088fd96) = 4;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 1;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0;

    r->esi = TableEntry(GP4_ARRAY(uint32_t, 0x00891bd0), 0x20u);
    gp4::cdecl_<0x0051693f, void>();

    GP4_GLOBAL(uint8_t, 0x0088fd9f) = saved9f;
    GP4_GLOBAL(uint8_t, 0x0088fda0) = savedA0;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = saved96;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = saved97;

    r->eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(saved97) << 8) | saved96;
}
GP4_IMPL(0x00516bb2, FUN_00516bb2, "regs(eax:u32) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
