#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041d996
void __cdecl FUN_0041d996(gp4::Regs* r) {
    gp4::call_regs(0x0041de65, *r);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(r->eax))
        * GP4_GLOBAL(int32_t, 0x00627bb4);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // SHRD at count 14 has architecturally undefined AF and OF; these
    // values model the pinned Unicorn oracle's shift implementation.
    const uint32_t flags = ((low >> 13) & 1u) | ((~parity & 1u) << 2)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041d996, FUN_0041d996, "regs() -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=layout=state_0041d996")
