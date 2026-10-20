#include <gp4/gp4.h>
#include <gp4/flags.h>

// The compiler would fold `address - address` into a constant and hide both globals from the shape audit.
static __declspec(noinline) uint32_t addrOf(const uint8_t* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// pushal wrapper: the signed dword at eax + (0x0066d830 - 0x0066d810) (the field +0x20 of the
// record eax points at) moves back by the step [0x0088f604]; when that goes negative it wraps by
// adding the span [0x007a8e2c] - [0x007a8e28]. The result is stored back. popal restores every
// register; the flags are those of the last subtraction (the step, or the wrap's `sub`).
// FUNCTION: GP4 0x00512d3f
void __cdecl FUN_00512d3f(gp4::Regs* r) {
    const uint32_t slot = r->eax + addrOf(GP4_ARRAY(uint8_t, 0x0066d830)) - addrOf(GP4_ARRAY(uint8_t, 0x0066d810));
    uint32_t value = GP4_FIELD(uint32_t, slot, 0);
    const uint32_t step = GP4_GLOBAL(uint32_t, 0x0088f604);
    uint32_t arith = gp4::flags::sub32(value, step);
    value -= step;
    if ((value & 0x80000000u) != 0u) {
        value += GP4_GLOBAL(uint32_t, 0x007a8e2c);
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a8e28);
        arith = gp4::flags::sub32(value, base);
        value -= base;
    }
    GP4_FIELD(uint32_t, slot, 0) = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512d3f, FUN_00512d3f, "regs(eax:ptr[0x28]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
