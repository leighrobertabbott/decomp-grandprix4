#include <gp4/gp4.h>

struct Unknown_0062a970 {
    int32_t Unknown00;
    uint32_t Unknown04;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00431f6e
void __cdecl FUN_00431f6e(gp4::Regs* r) {
    const uint32_t index = r->eax;
    uint32_t edx = r->edx;
    Unknown_0062a970* entry = GP4_ARRAY(Unknown_0062a970*, 0x0062a970)[index];

    edx += static_cast<uint32_t>(entry->Unknown00);
    uint32_t count = entry->Unknown04;

    // "or ecx, ecx" leaves CF=0, OF=0, AF=0 and sets ZF/PF/SF from the count.
    uint32_t flags = parityFlag8(count)
        | (count == 0u ? 0x40u : 0u)
        | ((count & 0x80000000u) != 0u ? 0x80u : 0u);
    uint32_t ax = r->eax & 0xffffu;

    while (count != 0u) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx));
        *reinterpret_cast<uint16_t*>(record + 0x16) = 0;
        *reinterpret_cast<uint16_t*>(record + 0x18) = 0;
        ax = *reinterpret_cast<uint16_t*>(record + 0x15c);
        *reinterpret_cast<uint16_t*>(record + 0xa6) = static_cast<uint16_t>(ax);
        flags = additionFlags32(edx, 0x160u);
        edx += 0x160u;
        count--;
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ecx = 0;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431f6e, FUN_00431f6e, "regs(eax:u32[0..3], edx:ptr[0x600]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00431f6e")
