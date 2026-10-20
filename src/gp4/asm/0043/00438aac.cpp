#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit logic result (or / and / test; CF = OF = AF = 0).
static inline uint32_t logicFlags32(uint32_t value) {
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~0x8d5u) | flags;
}

// imul dword [mem] ; shrd eax, edx, 14 : eax = low 32 bits of (eax * factor) >> 14, edx = high half.
static inline void mulQ14(gp4::Regs& s, int32_t factor) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(s.eax)) * factor;
    s.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    s.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// One roll of the dice: 0x00401462 returns a pseudo-random byte in al; true when it is below the
// threshold [0x0062a7bc]. The flags left behind are those of the final `cmp`.
static inline bool rollBelowThreshold(gp4::Regs& s) {
    gp4::call_regs(0x00401462, s);
    s.eax &= 0xffu;
    const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x0062a7bc);
    setFlags(s, cmpFlags32(s.eax, threshold));
    return static_cast<int32_t>(s.eax) < static_cast<int32_t>(threshold);
}

// FUNCTION: GP4 0x00438aac
// pushal wrapper over a car record at esi. For each of the four wheels/axes k the dword
// +0x2ec+4k is scaled by [0x62a7c0] (and first by [0x62a7c4] when bit 2 of +0xcc is set), both as
// 18.14 fixed point; the scaled value is compared with five threshold pairs (0x62a71c[k] gates
// them all). Each threshold reached rolls the random byte against [0x62a7bc]: the first calls
// 0x00410841 (with eax = [0x601e8c], ecx = k), the others collect the bit masks 0x62a760 /
// 0x62a780 / 0x62a740 / 0x62a7a0 (stride 8) into ebp. A non-empty mask is expanded by 0x00438a80
// and or-ed into dword +0x40 (when that is still zero, only if 0x004378fc leaves CF clear).
// popal restores every register; the flags are those of the last flag writer.
void __cdecl FUN_00438aac(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.ebp = 0;
    for (s.ecx = 0; ; ) {
        s.eax = GP4_FIELD(uint32_t, car, 0x2ec + s.ecx * 4);
        if ((GP4_FIELD(uint8_t, car, 0xcc) & 4u) != 0u) {
            mulQ14(s, GP4_GLOBAL(int32_t, 0x0062a7c4));
        }
        mulQ14(s, GP4_GLOBAL(int32_t, 0x0062a7c0));
        s.edi = s.eax;

        if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a71c)[s.ecx]) {
            setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a71c)[s.ecx]));
            if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a72c)[s.ecx]) {
                setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a72c)[s.ecx]));
                if (rollBelowThreshold(s)) {
                    s.eax = GP4_GLOBAL(uint32_t, 0x00601e8c);
                    gp4::call_regs(0x00410841, s);
                }
            }
            setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a75c)[s.ecx * 2]));
            if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a75c)[s.ecx * 2]) {
                if (rollBelowThreshold(s)) {
                    s.ebp |= GP4_ARRAY(uint32_t, 0x0062a760)[s.ecx * 2];
                }
            }
            setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a77c)[s.ecx * 2]));
            if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a77c)[s.ecx * 2]) {
                if (rollBelowThreshold(s)) {
                    s.ebp |= GP4_ARRAY(uint32_t, 0x0062a780)[s.ecx * 2];
                }
            }
            setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a73c)[s.ecx * 2]));
            if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a73c)[s.ecx * 2]) {
                if (rollBelowThreshold(s)) {
                    s.ebp |= GP4_ARRAY(uint32_t, 0x0062a740)[s.ecx * 2];
                }
            }
            setFlags(s, cmpFlags32(s.edi, GP4_ARRAY(uint32_t, 0x0062a79c)[s.ecx * 2]));
            if (static_cast<int32_t>(s.edi) >= GP4_ARRAY(int32_t, 0x0062a79c)[s.ecx * 2]) {
                if (rollBelowThreshold(s)) {
                    s.ebp |= GP4_ARRAY(uint32_t, 0x0062a7a0)[s.ecx * 2];
                }
            }
        }

        s.ecx += 1u;
        if (s.ecx == 4u) {
            setFlags(s, 0x44u);  // cmp ecx, 4 (equal)
            break;
        }
    }

    s.eax = s.ebp;
    setFlags(s, logicFlags32(s.eax));
    if (s.eax != 0u) {
        gp4::call_regs(0x00438a80, s);
        const uint32_t pending = GP4_FIELD(uint32_t, car, 0x40);
        setFlags(s, logicFlags32(pending));
        bool update = true;
        if (pending == 0u) {
            gp4::call_regs(0x004378fc, s);
            update = (s.eflags & 1u) == 0u;
        }
        if (update) {
            const uint32_t merged = GP4_FIELD(uint32_t, car, 0x40) | s.eax;
            GP4_FIELD(uint32_t, car, 0x40) = merged;
            setFlags(s, logicFlags32(merged));
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00438aac, FUN_00438aac, "regs(esi:ptr[0x420]:layout=car_00438aac+rec_004105c7) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_004105c7+state_00437888+gate_004378fc")
