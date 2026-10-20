#include <gp4/gp4.h>
#include <gp4/flags.h>

// Calls one of two empty (`ret`) hooks: the first when both selector bytes [0x007aafd3] and [0x0082456a] are 0xff
// and the low three bits of the dword [0x0081e5bc] are below 6, the second otherwise. On the path that reaches the
// dword test, eax is loaded with it and its low byte is masked with 7. The flags are those of the last compare
// executed (the empty hooks keep them).
static __declspec(noinline) void callHookA(gp4::Regs& c) {
    gp4::call_regs(0x0054bac0, c);
}

static __declspec(noinline) void callHookB(gp4::Regs& c) {
    gp4::call_regs(0x0054bad2, c);
}

// FUNCTION: GP4 0x0054ba96
void __cdecl FUN_0054ba96(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint8_t first = GP4_GLOBAL(uint8_t, 0x007aafd3);
    uint32_t arith = gp4::flags::sub8(first, 0xffu);
    bool hookA = false;
    if (first == 0xffu) {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x0082456a);
        arith = gp4::flags::sub8(second, 0xffu);
        if (second == 0xffu) {
            const uint32_t word = GP4_GLOBAL(uint32_t, 0x0081e5bc);
            const uint8_t low = static_cast<uint8_t>(word & 7u);
            c.eax = (word & 0xffffff00u) | low;
            arith = gp4::flags::sub8(low, 6u);
            hookA = low < 6u;
        }
    }
    c.eflags = gp4::flags::merge(c.eflags, arith);
    if (hookA) {
        callHookA(c);
    } else {
        callHookB(c);
    }
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054ba96, FUN_0054ba96, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
