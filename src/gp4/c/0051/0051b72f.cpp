#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t AddressOf(const uint8_t* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// pushal wrapper with no memory effect: edi starts at 0x7c4d5c + 6*0x70*3/3... = 0x7c4d5c + 0x70*3*2
// and the loop adds 6 twenty-four times. popal restores every register, so only the flags of the
// last `add edi, 6` survive.
// FUNCTION: GP4 0x0051b72f
void __cdecl FUN_0051b72f(gp4::Regs* r) {
    uint32_t edi = 0x70u;
    edi = edi + edi * 2u;
    edi = edi << 1;
    edi += AddressOf(GP4_ARRAY(uint8_t, 0x007c4d5c));
    uint32_t last = edi;
    for (uint32_t ecx = 0x18u; ecx != 0u; --ecx) {
        last = edi;
        edi += 6u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(last, 6u));
}
GP4_IMPL(0x0051b72f, FUN_0051b72f, "regs() -> (cf, pf, af, zf, sf, of)")
