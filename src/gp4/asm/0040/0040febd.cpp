#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// (a * b) >> 14 as "imul; shrd eax, edx, 14" leaves it in eax
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> 14);
}

// FUNCTION: GP4 0x0040febd
// Computes the two Q14 offsets DAT_00637f40 / DAT_00637f44 from the record at esi and the Q14
// factor in ecx: after 0x0040fdfb has produced DAT_00637f58/5c from ecx, with V = dword [esi+0x164]
// (also copied to DAT_006258e8) and W = word [esi+0x16a] (replaced first by the low word of
// DAT_006258e4 when that is nonzero):
//   DAT_00637f40 = (DAT_00637f58 - (((V * (0x4000 - W)) >> 14) * ecx >> 14)) * DAT_00601c74 >> 14
//   DAT_00637f44 = (DAT_00637f5c - (((V * W) >> 14) * ecx >> 14)) * DAT_00601c74 >> 14
// With bit 0 of [esi+0x8d] set both are cleared instead. eax/edx/flags are those of the last shrd
// (or of the bit test in the cleared case); ebx, ebp and ecx are preserved.
void __cdecl FUN_0040febd(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if ((GP4_FIELD(uint8_t, rec, 0x8d) & 1u) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00637f40) = 0;
        GP4_GLOBAL(uint32_t, 0x00637f44) = 0;
        // test byte, 1 with a result of 1: no arithmetic flag set
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    gp4::Regs call = *r;
    gp4::call_regs(0x0040fdfb, call);   // ecx is the factor; ecx itself is unchanged by it
    const int32_t factor = static_cast<int32_t>(r->ecx);

    const int32_t v = GP4_FIELD(int32_t, rec, 0x164);
    GP4_GLOBAL(int32_t, 0x006258e8) = v;
    if (GP4_GLOBAL(uint32_t, 0x006258e4) != 0u) {
        GP4_FIELD(uint16_t, rec, 0x16a) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x006258e4));
    }
    const int32_t w = GP4_FIELD(uint16_t, rec, 0x16a);

    const int32_t gain = GP4_GLOBAL(int32_t, 0x00601c74);

    const uint32_t first = mulShr14(static_cast<int32_t>(mulShr14(v, 0x4000 - w)), factor);
    const uint32_t offsetX = GP4_GLOBAL(uint32_t, 0x00637f58) - first;
    GP4_GLOBAL(uint32_t, 0x00637f40) = mulShr14(static_cast<int32_t>(offsetX), gain);

    const uint32_t second = mulShr14(static_cast<int32_t>(mulShr14(v, w)), factor);
    const uint32_t offsetZ = GP4_GLOBAL(uint32_t, 0x00637f5c) - second;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(offsetZ)) * gain;
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    GP4_GLOBAL(uint32_t, 0x00637f44) = result;

    // shrd eax, edx, 14: CF = last bit out, AF = 0 (pinned oracle model), OF = bit31(prev ^ result)
    const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    const uint32_t flags = (prev & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((prev ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040febd, FUN_0040febd, "regs(ecx:i32, esi:ptr[0x310]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
