#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit DEC (CF untouched, so it is left out).
static inline uint32_t decFlags32(uint32_t before) {
    const uint32_t result = before - 1u;
    return byteParity(result)
        | ((before & 0xfu) == 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (before == 0x80000000u ? 0x800u : 0u);
}

// Bisection on the engine word [0x00602d70] (start 0x60000): 0x00404f99 is run for the target
// [0x00603088] = ([0x00602d7c] * [0x006f2bec]) >> 4 and 0x00405759 measures the result; the word is halved
// towards the previous value until two successive measurements differ by less than 0x4000 or the counter
// [0x00622434] runs out. Returns the last measurement in eax (0x770000 when [0x00602d7c] is zero); ebx, ecx,
// edx and edi are pushed and restored, the flags are those of the last cmp / dec.
// FUNCTION: GP4 0x004056c9
void __cdecl FUN_004056c9(gp4::Regs* r) {
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00602d7c);
    if (scale == 0) {
        r->eax = 0x00770000u;
        // test dword [0x602d7c], -1 with a zero value: PF and ZF only
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const int64_t product = static_cast<int64_t>(scale) * GP4_GLOBAL(int32_t, 0x006f2bec);
    const int32_t target = static_cast<int32_t>(static_cast<uint32_t>(product)) >> 4;
    GP4_GLOBAL(int32_t, 0x00603088) = target;
    uint32_t counter = GP4_GLOBAL(uint32_t, 0x00622434);
    GP4_GLOBAL(int32_t, 0x00602d70) = 0x00060000;

    gp4::call_regs(0x00405759, c);
    c.ebx = c.eax;
    c.edx = GP4_GLOBAL(uint32_t, 0x00603088);
    gp4::call_regs(0x00404f99, c);
    gp4::call_regs(0x00405759, c);

    uint32_t flags;
    for (;;) {
        const uint32_t previous = GP4_GLOBAL(uint32_t, 0x00602d70);
        c.ebx = c.eax;
        c.edx = GP4_GLOBAL(uint32_t, 0x00603088);
        gp4::call_regs(0x00404f99, c);
        c.eax = previous;
        c.edx = GP4_GLOBAL(uint32_t, 0x00602d70);
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(c.eax + c.edx) >> 1);
        GP4_GLOBAL(uint32_t, 0x00602d70) = c.eax;
        gp4::call_regs(0x00405759, c);

        uint32_t gap = c.eax - c.ebx;
        if ((gap & 0x80000000u) != 0u) {
            gap = 0u - gap;
        }
        flags = cmpFlags32(gap, 0x4000u);
        if (static_cast<int32_t>(gap) < 0x4000) {
            break;
        }
        flags = decFlags32(counter) | (flags & 1u);
        counter -= 1u;
        if (counter == 0u) {
            break;
        }
    }

    r->eax = c.eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004056c9, FUN_004056c9, "regs(esi:ptr[0x118]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00404ed0+state_00404f33+engine_00413606+drive_004054e6+lanes_00429fcd")
