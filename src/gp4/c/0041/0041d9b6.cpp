#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041d9b6
void __cdecl FUN_0041d9b6(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi));
    gp4::call_regs(0x0041de65, *r);
    const int64_t firstProduct = static_cast<int64_t>(static_cast<int32_t>(r->eax))
        * GP4_GLOBAL(int32_t, 0x00627bb8);
    const uint32_t firstTerm = static_cast<uint32_t>(static_cast<uint64_t>(firstProduct) >> 14);
    const int64_t secondProduct = static_cast<int64_t>(static_cast<int32_t>(firstTerm))
        * GP4_GLOBAL(int32_t, 0x0062d284);
    const uint32_t secondTerm = static_cast<uint32_t>(static_cast<uint64_t>(secondProduct) >> 14);
    const uint32_t factor = (static_cast<uint32_t>(GP4_FIELD(const uint8_t, record, 0x65)) * 4u
        + GP4_GLOBAL(uint32_t, 0x0062d288)) << 10;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(secondTerm))
        * static_cast<int32_t>(factor);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // Architecturally undefined SHRD AF/OF at count14 follow the pinned
    // Unicorn oracle's penultimate/final shifted-value comparison.
    const uint32_t flags = ((low >> 13) & 1u) | ((~parity & 1u) << 2)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041d9b6, FUN_0041d9b6, "regs(edi:ptr[0x6d]:bytes) -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=layout=state_0041d9b6")


