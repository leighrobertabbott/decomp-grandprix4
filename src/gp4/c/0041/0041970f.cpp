#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit SUB / CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// `mov eax, [0x6228b4] ; mul dword [0x6228ac] ; mov [0x60384c], eax`: edx:eax = step * substeps.
static inline void loadSpan(gp4::Regs& c) {
    const uint64_t product = static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x006228b4)) * GP4_GLOBAL(uint32_t, 0x006228ac);
    c.eax = static_cast<uint32_t>(product);
    c.edx = static_cast<uint32_t>(product >> 32);
    GP4_GLOBAL(uint32_t, 0x0060384c) = c.eax;
}

// Splits the time span [0x0060384c] into [0x006228ac]-sized steps (rounded) and runs the car update chain once
// per chunk of at most [0x006228b0] steps, with [0x0060384c] set to chunk * [0x006228ac] for each pass; the span
// is restored at the end. The loop ends when the remaining count [0x006228b8] reaches zero, so eax = 0 and the
// flags are those of that final subtraction; the other registers are what the last callee left.
// FUNCTION: GP4 0x0041970f
void __cdecl FUN_0041970f(gp4::Regs* r) {
    const uint32_t span = GP4_GLOBAL(uint32_t, 0x0060384c);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t substeps = GP4_GLOBAL(uint32_t, 0x006228ac);
    c.ebp = substeps;
    uint32_t remainder = 0;
    const uint32_t total = gp4::div64(static_cast<uint64_t>(span + (substeps >> 1)), substeps, &remainder);
    c.eax = total;
    c.edx = remainder;
    GP4_GLOBAL(uint32_t, 0x006228bc) = total;
    GP4_GLOBAL(uint32_t, 0x006228b8) = total;
    GP4_GLOBAL(uint32_t, 0x006228b4) = GP4_GLOBAL(uint32_t, 0x006228b0);
    loadSpan(c);

    gp4::call_regs(0x00419666, c);
    gp4::call_regs(0x004195e1, c);
    gp4::call_regs(0x00416708, c);
    gp4::call_regs(0x00416738, c);
    GP4_GLOBAL(uint8_t, 0x006228c0) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x006228a8) = 0u;
    gp4::call_regs(0x00415841, c);

    uint32_t flags;
    for (;;) {
        loadSpan(c);
        gp4::call_regs(0x0041bee6, c);
        GP4_GLOBAL(uint8_t, 0x00622485) = 0xffu;
        gp4::call_regs(0x0041a0d0, c);
        gp4::call_regs(0x00419b84, c);
        gp4::call_regs(0x0041a099, c);
        gp4::call_regs(0x00415841, c);
        gp4::call_regs(0x00416b58, c);
        gp4::call_regs(0x0041c31e, c);

        GP4_GLOBAL(uint32_t, 0x006228a8) = GP4_GLOBAL(uint32_t, 0x006228a8) + 1u;
        const uint32_t left = GP4_GLOBAL(uint32_t, 0x006228b8);
        const uint32_t right = GP4_GLOBAL(uint32_t, 0x006228b4);
        const uint32_t remaining = left - right;
        flags = cmpFlags32(left, right);
        c.eax = remaining;
        GP4_GLOBAL(uint32_t, 0x006228b8) = remaining;
        if (remaining == 0u) {
            break;
        }
        if (static_cast<int32_t>(remaining) > GP4_GLOBAL(int32_t, 0x006228b0)) {
            c.eax = GP4_GLOBAL(uint32_t, 0x006228b0);
        }
        GP4_GLOBAL(uint32_t, 0x006228b4) = c.eax;
        GP4_GLOBAL(uint8_t, 0x006228c0) = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x0060384c) = span;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041970f, FUN_0041970f, "regs(esi:ptr[0x41c]:layout=rec_004195e1+rec_0041b950+car_00416ec9) -> (eax, ebx, ecx, edx, ebp, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004195e1+state_00416708+state_0041b950+state_00416dec+state_00416fce+sums_00419f9e+curves_0062ecb0+dw_0041c027+state_0041bce9+state_0041bd56+state_0041bbef+state_0041bd28+div_0041bc3a+state_0041befb+state_0041c145+state_004209ae+bounds_0041970f+const_006258b8+aero_00637f38; maxinsn=16000000")
