#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Passes the 16-bit sum of the words +0x18 and +0x00 (eax, sign-extended) and the word +0x16 (edx,
// sign-extended) of the record in edi to 0x0041f02b. The flags are those of the 16-bit add.
// FUNCTION: GP4 0x0041f1ad
void __cdecl FUN_0041f1ad(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t left = GP4_FIELD(uint16_t, record, 0x18);
    const uint32_t right = GP4_FIELD(uint16_t, record, 0);
    const uint32_t sum = (left + right) & 0xffffu;
    const uint32_t flags = ((left + right) > 0xffffu ? 1u : 0u)
        | parityFlag(sum)
        | (((left ^ right ^ sum) & 0x10u) != 0u ? 0x10u : 0u)
        | (sum == 0u ? 0x40u : 0u)
        | ((sum & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ sum) & 0x8000u) != 0u ? 0x800u : 0u);

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(sum)));
    call.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x16)));
    call.eflags = (call.eflags & ~0x8d5u) | flags;
    gp4::call_regs(0x0041f02b, call);
    *r = call;
}
GP4_IMPL(0x0041f1ad, FUN_0041f1ad, "regs(edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=layout=surf_0041f02b")
