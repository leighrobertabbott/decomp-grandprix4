#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte [0x7c2ea9] is 0x80, takes the key byte at 0x7c31dc + [0xa69878] (the car order
// array) and asks 0x00408359 for the car record whose number byte +0x7c equals it. If one is found
// ([0xa69878] becomes its (+0x7c & 0x3f) - 1) and its byte +0x8d has bit 1 set, [0xa6987c] = 0xe;
// every other path stores 0x10 there. pushal/popal keeps every register; the flags are those of
// the last writer: `cmp [0x7c2ea9], 0x80`, the flags 0x408359 returned (not found), or the
// closing `test byte [esi+0x8d], 2`.
// FUNCTION: GP4 0x005162fd
void __cdecl FUN_005162fd(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = gp4::flags::sub8(mode, 0x80u);
    bool found = false;
    if (mode == 0x80) {
        const uint32_t index = GP4_GLOBAL(uint32_t, 0x00a69878);
        const uint32_t key = GP4_ARRAY(uint8_t, 0x007c31dc)[index];
        gp4::Regs c = {};
        c.eax = (index & 0xffffff00u) | key;
        c.eflags = r->eflags;
        gp4::call_regs(0x00408359, c);
        flags = c.eflags & gp4::flags::ARITH;
        if ((c.eflags & gp4::flags::CF) != 0u) {
            const uint32_t record = c.eax;
            GP4_GLOBAL(uint32_t, 0x00a69878) = (GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu) - 1u;
            const uint8_t bits = GP4_FIELD(uint8_t, record, 0x8d);
            flags = gp4::flags::logic8(bits & 2u);
            found = (bits & 2u) != 0u;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = found ? 0x0eu : 0x10u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005162fd, FUN_005162fd, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+carno_0066ff6c+idx_005162fd")
