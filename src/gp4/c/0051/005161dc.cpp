#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call target so the compiler keeps two distinct call sites.
static __declspec(noinline) void CallCarSlot(gp4::Regs& c) {
    gp4::call_regs(0x00408309, c);
}
static __declspec(noinline) void CallOrderSearch(gp4::Regs& c) {
    gp4::call_regs(0x00516d07, c);
}

// pushal/popal routine that stores a 1-based position of the selected car [0x679d30] into
// [0xa6987c]: when the mode byte [0x7c2ea9] is 0x80 the value of 0x408309 (byte +0x415 >> 1, plus
// one); otherwise the low byte of the order-list search 0x516d07 (eax = 0 on entry) plus one (16-bit
// add, so 0xff gives 0x100). Every register is preserved; the flags are those the chosen branch
// leaves (the callee's, or the `add ax, 1`).
// FUNCTION: GP4 0x005161dc
void __cdecl FUN_005161dc(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        CallCarSlot(c);
        flags = c.eflags;
        GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    } else {
        c.eax = 0u;
        CallOrderSearch(c);
        const uint32_t index = c.eax & 0xffu;
        flags = gp4::flags::add16(index, 1u);
        GP4_GLOBAL(uint32_t, 0x00a6987c) = static_cast<uint16_t>(index + 1u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005161dc, FUN_005161dc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30+order_00675958")
