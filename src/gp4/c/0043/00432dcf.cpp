#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t res) {
    return (res < a ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ res) & (b ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x00432dcf
// Walks the 22 records (stride 0x41c) at 0x0066fef0. When the mask at
// (DAT_0062ff64 + 0x10) is non-zero, each record whose index (byte +0x7c & 0x3f,
// minus 1, clamped to 21) selects a set bit of the mask gets bit 3 set in its
// byte at +0x25f. Registers left behind: eax = last index, ebp = mask,
// esi = end of the table, ecx = 0 (or the mask-zero early-out values).
void __cdecl FUN_00432dcf(gp4::Regs* r) {
    const uint32_t mask = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062ff64) + 0x10u));
    r->ebp = mask;
    if (mask == 0u) {
        // flags of: or ebp, ebp
        r->eax = GP4_GLOBAL(uint32_t, 0x0062ff64) + 0x10u;
        r->eflags = (r->eflags & ~0x8D5u) | parityFlag(0u) | 0x40u;
        return;
    }
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x0066fef0);
    int32_t index = 0;
    for (int i = 0; i < 22; ++i) {
        index = (int32_t)(rec[0x7c] & 0x3fu) - 1;
        if (index > 0x15) {
            index = 0x15;
        }
        if ((mask >> ((uint32_t)index & 31u)) & 1u) {
            rec[0x25f] |= 8;
        }
        rec += 0x41c;
    }
    const uint32_t esi = (uint32_t)(uintptr_t)rec;
    r->eax = (uint32_t)index;
    r->ecx = 0;
    r->esi = esi;
    // flags of the final "add esi, 0x41c"
    r->eflags = (r->eflags & ~0x8D5u) | addFlags(esi - 0x41cu, 0x41cu, esi);
}
GP4_IMPL(0x00432dcf, FUN_00432dcf, "regs(ecx:u32, esi:u32) -> (eax, ecx, esi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00432dcf")
