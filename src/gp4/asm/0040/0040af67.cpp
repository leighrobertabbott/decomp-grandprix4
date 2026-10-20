#include <gp4/gp4.h>

extern "C" void __cdecl __debugbreak(void);

// FUNCTION: GP4 0x0040af67
void __cdecl FUN_0040af67(gp4::Regs* r) {
    // Finds the first group of 8 consecutive 0x160-byte records (group stride 8 records,
    // starting at record `eax` of the block at [0x891504]) whose byte +0x48 has no bit
    // of mask 0x30 set. Leaves edi at that group. Runs out of range or after 24 groups
    // -> int3.
    const uint64_t scaled = static_cast<uint64_t>(r->eax) * 0x160u;   // mul edx: edx:eax
    const uint32_t scaledLow = static_cast<uint32_t>(scaled);
    const uint32_t scaledHigh = static_cast<uint32_t>(scaled >> 32);
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00891504) + scaledLow;
    uint32_t dx = 0x18;
    uint32_t limit;
    uint32_t al = 0;
    for (;;) {
        limit = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x5800u;
        if (edi >= limit) {
            __debugbreak();
            return;
        }
        uint32_t ah = 0;
        const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(edi));
        for (int i = 0; i < 8; ++i) {
            al = rec[0x48] & 0x30u;
            ah |= al;
            rec += 0x160;
        }
        if (ah == 0) break;
        edi += 0xb00u;
        dx = (dx - 1u) & 0xffffu;
        if (dx == 0) {
            __debugbreak();
            return;
        }
    }
    r->eax = (scaledLow & 0xffff0000u) | al;              // ah == 0 here
    r->ecx = 0;
    r->edx = (scaledHigh & 0xffff0000u) | dx;
    r->ebp = limit;
    r->edi = edi;
    // or ah, ah with ah == 0: CF = OF = AF = 0, ZF = 1, PF = 1, SF = 0
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040af67, FUN_0040af67, "regs(eax:u32[0..63]) -> (eax, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=ring_0040af67")
