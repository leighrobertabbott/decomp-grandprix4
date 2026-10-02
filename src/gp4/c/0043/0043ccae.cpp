#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ccae
void __cdecl FUN_0043ccae(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ecx = 0;
    do {
        // Two draws from the shift-register generator at 0x00401462: high byte, then low byte.
        r->eax = eax;
        r->ecx = ecx;
        r->edx = edx;
        gp4::call_regs(0x00401462, *r);
        const uint32_t high = (r->eax & 0xffu) << 8;
        r->eax = r->eax & 0xffu;
        gp4::call_regs(0x00401462, *r);
        const uint32_t noise = ((r->eax & 0xffu) | high) & 0x3fffu;
        const int32_t scale = static_cast<int32_t>(0x4000u - GP4_GLOBAL(uint32_t, 0x00622414));
        const int64_t first = static_cast<int64_t>(static_cast<int32_t>(noise)) * scale;
        const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
        const uint32_t weight = GP4_ARRAY(uint16_t, 0x0062eb78)[ecx * 2];
        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * static_cast<int64_t>(weight);
        eax = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
        edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
        GP4_ARRAY(uint16_t, 0x0066d7aa)[ecx] = static_cast<uint16_t>(eax);
        ecx++;
    } while (ecx != 0x16);
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    // Last flag writer: cmp ecx, 0x16 with equal operands -> ZF and PF set, the rest clear.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043ccae, FUN_0043ccae, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
