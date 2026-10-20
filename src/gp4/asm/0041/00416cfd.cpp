#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of SHRD r32, r32, count (count > 1) in the pinned oracle model.
static inline uint32_t shrdFlags32(uint32_t low, uint32_t high, uint32_t count) {
    const uint64_t joined = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t result = static_cast<uint32_t>(joined >> count);
    const uint32_t previous = static_cast<uint32_t>(joined >> (count - 1));
    return (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// "imul ; shrd eax, edx, 14": low dword of the signed 64-bit product >> 14.
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint32_t mulLow(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>(static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b)));
}

static inline uint32_t mulHigh(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b))) >> 32);
}

// Picks the two Q14 output values DAT_00637f38 / DAT_00637f3c (eax / edx) from the car record in esi.
// byte +0xe4 in 2..5 (signed): both = DAT_00601cac. Otherwise, with bit 5 of +0x7a set: both = DAT_00601ca4
// when bit 1 of +0x6f is set, else the incoming eax / edx. Otherwise DAT_0060139c (or DAT_0062ecac when bit 0
// of +0x8d) is scaled by max(DAT_00638d30, DAT_0062fbec) (also stored as a word at +0x24a) and blended with
// the word at +0x1e2 as a Q14 fraction. Finally, when bit 4 of +0x8d is set, bit 3 of +0x162 clear and
// byte +0x84 == 9, both are multiplied by DAT_00601ff8 (Q14). ebp leaves holding the Q14 fraction state,
// ecx the unscaled reference; the flags are those of the last test/cmp/shrd.
// FUNCTION: GP4 0x00416cfd
void __cdecl FUN_00416cfd(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t ebp = GP4_FIELD(uint16_t, record, 0x1e2);

    const int8_t mode = GP4_FIELD(int8_t, record, 0xe4);
    if (mode >= 2 && mode <= 5) {
        eax = GP4_GLOBAL(uint32_t, 0x00601cac);
        edx = eax;
    } else if ((GP4_FIELD(uint8_t, record, 0x7a) & 0x20u) != 0u) {
        if ((GP4_FIELD(uint8_t, record, 0x6f) & 2u) != 0u) {
            eax = GP4_GLOBAL(uint32_t, 0x00601ca4);
            edx = eax;
        }
    } else {
        eax = GP4_GLOBAL(uint32_t, 0x0060139c);
        if ((GP4_FIELD(uint8_t, record, 0x8d) & 1u) != 0u) {
            eax = GP4_GLOBAL(uint32_t, 0x0062ecac);
        }
        edx = GP4_GLOBAL(uint32_t, 0x00638d30);
        if (static_cast<int32_t>(edx) < GP4_GLOBAL(int32_t, 0x0062fbec)) {
            edx = GP4_GLOBAL(uint32_t, 0x0062fbec);
        }
        GP4_FIELD(uint16_t, record, 0x24a) = static_cast<uint16_t>(edx);
        ecx = mulShr14(eax, edx);
        const uint32_t blended = mulShr14(ecx, ebp);
        ebp = 0x4000u - ebp;
        if (GP4_GLOBAL(uint8_t, 0x00638dc0) != 0u) {
            ecx = GP4_GLOBAL(uint32_t, 0x0060139c);
            if ((GP4_FIELD(uint8_t, record, 0x8d) & 1u) != 0u) {
                ecx = GP4_GLOBAL(uint32_t, 0x0062ecac);
            }
            ebp = 0x4000u;
        }
        eax = mulShr14(ecx, ebp);
        edx = blended;
    }

    uint32_t flags = testFlags8(GP4_FIELD(uint8_t, record, 0x8d) & 0x10u);
    if ((GP4_FIELD(uint8_t, record, 0x8d) & 0x10u) != 0u) {
        flags = testFlags8(GP4_FIELD(uint8_t, record, 0x162) & 8u);
        if ((GP4_FIELD(uint8_t, record, 0x162) & 8u) == 0u) {
            flags = subFlags8(GP4_FIELD(uint8_t, record, 0x84), 9u);
            if (GP4_FIELD(uint8_t, record, 0x84) == 9u) {
                const uint32_t scale = GP4_GLOBAL(uint32_t, 0x00601ff8);
                const uint32_t scaledEdx = mulShr14(edx, scale);
                const uint32_t low = mulLow(eax, scale);
                const uint32_t high = mulHigh(eax, scale);
                flags = shrdFlags32(low, high, 14u);
                eax = mulShr14(eax, scale);
                edx = scaledEdx;
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x00637f38) = eax;
    GP4_GLOBAL(uint32_t, 0x00637f3c) = edx;

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00416cfd, FUN_00416cfd, "regs(esi:ptr[0x252]:bytes, eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
