#include <gp4/gp4.h>

struct Unknown_00410841_Record {
    uint8_t Unknown00[0x2cc];
    uint32_t Divisor2cc[4];
    uint32_t Subtrahend2dc[4];
    uint8_t Unknown2ec[0x50];
    uint32_t Result33c[4];
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right` whose result is `result`.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// pushal ... popal routine: every register comes back unchanged; only the dword at
// record->Result33c[index] is written, and the flags are those of the last "sub eax, [mem]".
// target = DAT_00601df4[i] - (-eax + (DAT_00601d7c[i] << 10) / DAT_00601d9c[i]
//          + ((DAT_00601d7c[i] << 6) / record->Divisor2cc[i]) * 16) - record->Subtrahend2dc[i]
// FUNCTION: GP4 0x00410841
void __cdecl FUN_00410841(gp4::Regs* r) {
    const uint32_t offset = r->eax;
    const uint32_t index = r->ecx;
    Unknown_00410841_Record* record = reinterpret_cast<Unknown_00410841_Record*>(static_cast<uintptr_t>(r->esi));

    const uint32_t value = GP4_ARRAY(uint32_t, 0x00601d7c)[index];
    uint32_t remainder;

    uint32_t sum = 0u - offset;
    sum += gp4::div64(static_cast<uint64_t>(value) << 10, GP4_ARRAY(uint32_t, 0x00601d9c)[index], &remainder);
    const uint32_t scaled = gp4::div64(static_cast<uint64_t>(value) << 6, record->Divisor2cc[index], &remainder);
    sum += scaled << 4;

    const uint32_t partial = GP4_ARRAY(uint32_t, 0x00601df4)[index] - sum;
    const uint32_t subtrahend = record->Subtrahend2dc[index];
    const uint32_t result = partial - subtrahend;
    record->Result33c[index] = result;

    r->eflags = (r->eflags & ~0x8d5u) | subFlags(partial, subtrahend, result);
}
GP4_IMPL(0x00410841, FUN_00410841, "regs(eax:u32, ecx:u32[0..3], esi:ptr[0x358]:layout=rec_004105c7) -> (cf, pf, af, zf, sf, of); globals=layout=state_004105c7")
