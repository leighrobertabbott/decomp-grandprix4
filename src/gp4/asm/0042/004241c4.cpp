#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Destination block filled from the 0x00638030.. and 0x00623eb8.. state dwords.
struct Unknown_004241c4 {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    int32_t Unknown18;
    int32_t Unknown1C;
    int32_t Unknown20;
    int32_t Unknown24;
    int32_t Unknown28;
    int32_t Unknown2C;
    uint8_t Unknown30[0x28];
    int32_t Unknown58;
};

// FUNCTION: GP4 0x004241c4
// Snapshots the dwords at 0x00638040/44/48 (+0x00..+0x08), 0x0063804c/50/54 >> 8 (+0x18..+0x20),
// 0x00638034/30/38 (+0x0c..+0x14) and 0x00623eb8/bc/c0 (+0x24..+0x2c) into the block at esi and
// clears +0x58. Leaves eax = [0x00623ec0]; the flags are those of the last "sar eax, 8"
// (value [0x00638054]): CF = bit 7, AF = OF = 0, PF/ZF/SF from the result.
void __cdecl FUN_004241c4(gp4::Regs* r) {
    Unknown_004241c4* block = reinterpret_cast<Unknown_004241c4*>(static_cast<uintptr_t>(r->esi));

    block->Unknown00 = GP4_GLOBAL(int32_t, 0x00638040);
    block->Unknown04 = GP4_GLOBAL(int32_t, 0x00638044);
    block->Unknown08 = GP4_GLOBAL(int32_t, 0x00638048);
    block->Unknown18 = GP4_GLOBAL(int32_t, 0x0063804c) >> 8;
    block->Unknown1C = GP4_GLOBAL(int32_t, 0x00638050) >> 8;
    const int32_t last = GP4_GLOBAL(int32_t, 0x00638054);
    const int32_t shifted = last >> 8;
    block->Unknown20 = shifted;
    block->Unknown0C = GP4_GLOBAL(int32_t, 0x00638034);
    block->Unknown10 = GP4_GLOBAL(int32_t, 0x00638030);
    block->Unknown14 = GP4_GLOBAL(int32_t, 0x00638038);
    block->Unknown24 = GP4_GLOBAL(int32_t, 0x00623eb8);
    block->Unknown28 = GP4_GLOBAL(int32_t, 0x00623ebc);
    const int32_t tail = GP4_GLOBAL(int32_t, 0x00623ec0);
    block->Unknown2C = tail;
    block->Unknown58 = 0;

    const uint32_t result = static_cast<uint32_t>(shifted);
    const uint32_t flags = ((static_cast<uint32_t>(last) >> 7) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u);

    r->eax = static_cast<uint32_t>(tail);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004241c4, FUN_004241c4, "regs(esi:ptr[0x60]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
