#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408964
void __cdecl FUN_00408964(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    gp4::Regs callee = *r;
    gp4::call_regs(0x00401462, callee);
    const uint32_t factor = (callee.eax & 0xffu) << 6;
    const uint32_t folded = static_cast<int32_t>(incomingEax) < 0x2000
        ? incomingEax + 0x2000u : 0x6000u - incomingEax;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(folded))
        * static_cast<int32_t>(factor);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // SHRD at count 14 has architecturally undefined AF and OF. These
    // values model the pinned Unicorn oracle's shift implementation.
    const uint32_t flags = ((low >> 13) & 1u) | ((~parity & 1u) << 2)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408964, FUN_00408964, "regs(eax:i32) -> (eax,cf,pf,af,zf,sf,of); globals=layout=state_0066d8c0")
