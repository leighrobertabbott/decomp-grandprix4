#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: looks up the car record for the current order entry. The key byte is
// 0x7c31dc[[0xa69878]] when the mode byte [0x7c2ea9] is 0x80 (then [0xa6987c] = (key & 0x3f) - 1), else
// 0x675958[[0xa69878]] ([0xa6987c] = (key - 1) & 0x3f); the dword with the key in its low byte is kept in
// [0xa6988c]. 0x00408359 finds the car record whose number byte +0x7c matches and 0x0040837c tests its state
// byte +0x7a; with both CF results set the record's bytes +0x70 and +0x74 and the dword
// 0x7c3208[[0xa6987c]] go to [0xa69880] / [0xa69884] / [0xa69888]; otherwise [0xa6987c] = 99999 and the three
// are cleared. popal restores every register; the flags are those of the last flag writer on the path (the
// callee that failed, or the closing `and eax, 0xff`).
// FUNCTION: GP4 0x0051624b
void __cdecl FUN_0051624b(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00a69878);
    uint32_t key;
    uint32_t slot;
    uint32_t atCall;
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        key = (index & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x007c31dc)[index];
        GP4_GLOBAL(uint32_t, 0x00a6988c) = key;
        slot = (key & 0x3fu) - 1u;
        atCall = gp4::flags::dec32(key & 0x3fu, gp4::flags::logic32(key & 0x3fu));
    } else {
        key = (index & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x00675958)[index];
        GP4_GLOBAL(uint32_t, 0x00a6988c) = key;
        slot = (key - 1u) & 0x3fu;
        atCall = gp4::flags::logic32(slot);
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = slot;

    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint32_t, 0x00a6988c);
    c.eflags = gp4::flags::merge(c.eflags, atCall);
    gp4::call_regs(0x00408359, c);
    bool found = (c.eflags & gp4::flags::CF) != 0u;
    if (found) {
        c.esi = c.eax;
        gp4::call_regs(0x0040837c, c);
        found = (c.eflags & gp4::flags::CF) != 0u;
    }

    uint32_t flags = c.eflags;
    if (found) {
        const uint32_t record = c.esi;
        GP4_GLOBAL(uint32_t, 0x00a69880) = GP4_FIELD(uint8_t, record, 0x70);
        GP4_GLOBAL(uint32_t, 0x00a69884) = GP4_ARRAY(uint32_t, 0x007c3208)[slot];
        const uint32_t tail = GP4_FIELD(uint8_t, record, 0x74);
        GP4_GLOBAL(uint32_t, 0x00a69888) = tail;
        flags = gp4::flags::merge(c.eflags, gp4::flags::logic32(tail));
    } else {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 0x1869fu;
        GP4_GLOBAL(uint32_t, 0x00a69880) = 0u;
        GP4_GLOBAL(uint32_t, 0x00a69884) = 0u;
        GP4_GLOBAL(uint32_t, 0x00a69888) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051624b, FUN_0051624b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+carno_0066ff6c+idx_005162fd+order_00675958")
