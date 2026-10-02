#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004086e4
void __cdecl FUN_004086e4(gp4::Regs* r) {
    // push edx / pop edx: edx is preserved around the call
    gp4::Regs inner = *r;
    gp4::call_regs(0x00401462, inner);
    const int32_t sample = static_cast<int8_t>(inner.eax & 0xffu);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x0062247d);
    const int64_t product = static_cast<int64_t>(sample) * static_cast<int64_t>(scale);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    // shrd eax, edx, 14 on the 64-bit product. CF is the last bit out; OF/AF follow the
    // pinned oracle model (AF=0, OF=bit31(previous ^ result)) for counts above one.
    const uint64_t joined = static_cast<uint64_t>(product);
    const uint32_t result = static_cast<uint32_t>(joined >> 14);
    const uint32_t previous = static_cast<uint32_t>(joined >> 13);
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    (void)high;
}
GP4_IMPL(0x004086e4, FUN_004086e4, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
