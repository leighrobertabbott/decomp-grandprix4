#include <gp4/gp4.h>

// The four 0x300-byte wheel records at 0x006380d8: only the fields used here.
struct Unknown_006380d8 {
    uint8_t Unknown00[0x1c];
    int32_t Unknown1C;
    uint8_t Unknown20[0x40 - 0x20];
    int32_t Unknown40;
    uint8_t Unknown44[0x48 - 0x44];
    int32_t Unknown48;
    uint8_t Unknown4C[0x1a0 - 0x4c];
    int32_t Unknown1A0;
    uint8_t Unknown1A4[0x258 - 0x1a4];
    int32_t Unknown258;
    uint8_t Unknown25C[0x260 - 0x25c];
    int32_t Unknown260;
    uint8_t Unknown264[0x268 - 0x264];
    uint8_t Unknown268;
    uint8_t Unknown269[0x300 - 0x269];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// low dword of the signed 64-bit product shifted right by N (imul ; shrd eax, edx, N).
template <int N>
static inline uint32_t mulShr(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> N);
}

// test r, -1 ; jns ; neg r
static inline uint32_t absolute(uint32_t value) {
    return static_cast<int32_t>(value) < 0 ? 0u - value : value;
}

// Magnitude limited (unsigned jb) to the dword 0x601c88, sign kept.
static inline uint32_t limitMagnitude(uint32_t value) {
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601c88);
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (!(magnitude < limit)) {
            magnitude = limit;
        }
        return 0u - magnitude;
    }
    if (!(value < limit)) {
        value = limit;
    }
    return value;
}

// FUNCTION: GP4 0x0041c31e
// Runs 1 << [0x6228c4] sub-steps with the time step [0x60384c] >> [0x6228c4] (restored on exit). Each
// sub-step calls 0x41c228 and then updates dword +0x40 of the four wheel records at 0x6380d8 against
// +0x1c, +0x48, +0x1a0, +0x258 and +0x260 with the per-wheel gains 0x601a80/0x601a90, limited in
// magnitude by [0x601c88], and stores +0x40 + +0x1c (through 0x41c300 when |+0x1c| < [0x601c8c] and
// |+0x40| < [0x601c90]) to the dword esi+0x39c+4k. Exit: ebx = 0x6380d8, ecx = 0, ebp = the old +0x40
// of wheel 3, eax/edx from wheel 3, flags of the final "dec ecx" reaching 0 (CF from the equal cmp).
void __cdecl FUN_0041c31e(gp4::Regs* r) {
    gp4::Regs c = *r;

    const uint32_t savedStep = GP4_GLOBAL(uint32_t, 0x0060384c);
    const uint32_t shift = GP4_GLOBAL(uint32_t, 0x006228c4) & 0x1fu;
    uint32_t count = 1u << shift;
    GP4_GLOBAL(uint32_t, 0x0060384c) = GP4_GLOBAL(uint32_t, 0x0060384c) >> shift;
    c.eax = count;

    do {
        c.ecx = count;
        c.esi = r->esi;
        gp4::call_regs(0x0041c228, c);

        uint32_t* out = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.esi + 0x39cu));
        Unknown_006380d8* wheel = reinterpret_cast<Unknown_006380d8*>(GP4_ARRAY(uint8_t, 0x006380d8));
        uint32_t k = 0;
        do {
            wheel->Unknown268 = 0;
            const uint32_t oldValue = static_cast<uint32_t>(wheel->Unknown40);
            const uint32_t step = GP4_GLOBAL(uint32_t, 0x0060384c);
            uint32_t eax = static_cast<uint32_t>(wheel->Unknown260) + static_cast<uint32_t>(wheel->Unknown1A0);
            uint32_t edx = c.edx;
            uint32_t ebp = c.ebp;

            bool direct;   // the path from 0x41c437
            bool towards = false;   // the path to 0x41c4ee
            bool done = false;      // straight to 0x41c587
            if (wheel->Unknown1A0 == 0 && static_cast<int32_t>(k) < 2) {
                direct = true;
            } else {
                direct = static_cast<int32_t>(eax ^ static_cast<uint32_t>(wheel->Unknown1A0)) >= 0;
            }

            if (!direct) {
                const uint32_t moved = absolute(mulShr<16>(wheel->Unknown258, GP4_GLOBAL(int32_t, 0x0060384c)));
                edx = absolute(static_cast<uint32_t>(wheel->Unknown40));
                if (!(static_cast<int32_t>(moved) > static_cast<int32_t>(edx))) {
                    edx -= moved;
                    if (wheel->Unknown40 < 0) {
                        edx = 0u - edx;
                    }
                    wheel->Unknown40 = static_cast<int32_t>(edx);
                    done = true;
                } else {
                    ebp = absolute(static_cast<uint32_t>(wheel->Unknown258));
                    if (ebp == 0u) {
                        ebp = 1u;
                    }
                    const uint32_t magnitude = absolute(static_cast<uint32_t>(wheel->Unknown40));
                    const uint32_t q = gp4::div64(static_cast<uint64_t>(magnitude) << 16, ebp, &edx);
                    wheel->Unknown40 = 0;
                    if (static_cast<int32_t>(q) >= GP4_GLOBAL(int32_t, 0x0060384c)) {
                        done = true;
                    } else {
                        GP4_GLOBAL(uint32_t, 0x0060384c) -= q;
                        const int32_t* gain = GP4_ARRAY(int32_t, 0x00601a80);
                        wheel->Unknown258 = static_cast<int32_t>(static_cast<uint32_t>(wheel->Unknown258)
                            + mulShr<14>(wheel->Unknown1A0, gain[k]));
                        towards = true;
                    }
                }
            } else {
                const uint32_t sum = absolute(static_cast<uint32_t>(wheel->Unknown260) + static_cast<uint32_t>(wheel->Unknown1A0));
                ebp = absolute(static_cast<uint32_t>(wheel->Unknown1A0));
                if (!(static_cast<int32_t>(ebp) > static_cast<int32_t>(sum))) {
                    towards = true;
                } else {
                    const uint32_t q = gp4::div64(static_cast<uint64_t>(sum) << 14, ebp, &edx);
                    const uint32_t share = mulShr<14>(static_cast<int32_t>(0x4000u - q), wheel->Unknown40);
                    ebp = absolute(share);
                    eax = mulShr<16>(wheel->Unknown258, GP4_GLOBAL(int32_t, 0x0060384c));
                    if (static_cast<int32_t>(eax) < 0) {
                        eax = 0u - eax;
                        if (!(eax < ebp)) {
                            eax = ebp;
                        }
                        eax = 0u - eax;
                    } else if (!(eax < ebp)) {
                        eax = ebp;
                    }
                    edx = static_cast<uint32_t>(wheel->Unknown40);
                    eax += edx;
                    edx ^= eax;
                    if (static_cast<int32_t>(edx) < 0) {
                        eax = 0u;
                    }
                    wheel->Unknown40 = static_cast<int32_t>(limitMagnitude(eax));
                    done = true;
                }
            }

            if (!done && towards) {
                const int32_t* gain = GP4_ARRAY(int32_t, 0x00601a80);
                const int32_t* damp = GP4_ARRAY(int32_t, 0x00601a90);
                eax = mulShr<14>(wheel->Unknown258, damp[k]);
                eax = mulShr<16>(static_cast<int32_t>(eax), GP4_GLOBAL(int32_t, 0x0060384c));
                eax += static_cast<uint32_t>(wheel->Unknown1C);
                eax += static_cast<uint32_t>(wheel->Unknown40);
                if (static_cast<int32_t>(eax ^ static_cast<uint32_t>(wheel->Unknown1C)) < 0
                    && static_cast<int32_t>(eax ^ static_cast<uint32_t>(wheel->Unknown48)) >= 0) {
                    const uint32_t partial = eax;
                    uint32_t back = mulShr<14>(wheel->Unknown48, gain[k]);
                    back = mulShr<14>(static_cast<int32_t>(back), static_cast<int32_t>(static_cast<uint32_t>(damp[k]) + 0x4000u));
                    back = 0u - back;
                    back = mulShr<16>(static_cast<int32_t>(back), GP4_GLOBAL(int32_t, 0x0060384c));
                    eax = back + partial;
                    if (static_cast<int32_t>(eax ^ static_cast<uint32_t>(wheel->Unknown1C)) >= 0) {
                        eax = 0u;
                    }
                }
                eax -= static_cast<uint32_t>(wheel->Unknown1C);
                wheel->Unknown40 = static_cast<int32_t>(limitMagnitude(eax));
            }

            // 0x41c587
            eax = static_cast<uint32_t>(wheel->Unknown40) + static_cast<uint32_t>(wheel->Unknown1C);
            edx = absolute(static_cast<uint32_t>(wheel->Unknown1C));
            if (static_cast<int32_t>(edx) < GP4_GLOBAL(int32_t, 0x00601c8c)) {
                edx = absolute(static_cast<uint32_t>(wheel->Unknown40));
                if (static_cast<int32_t>(edx) < GP4_GLOBAL(int32_t, 0x00601c90)) {
                    c.eax = eax;
                    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(wheel));
                    c.ecx = k;
                    c.edx = edx;
                    c.ebp = ebp;
                    gp4::call_regs(0x0041c300, c);
                    eax = c.eax;
                }
            }
            out[k] = eax;
            GP4_GLOBAL(uint32_t, 0x0060384c) = step;

            c.eax = eax;
            c.edx = edx;
            c.ebp = oldValue;
            ++k;
            ++wheel;
        } while (reinterpret_cast<uint8_t*>(wheel) != GP4_ARRAY(uint8_t, 0x00638cd8));
        c.ebx = 0x00638cd8u;
        --count;
    } while (count != 0u);

    GP4_GLOBAL(uint32_t, 0x0060384c) = savedStep;

    r->eax = c.eax;
    r->ebx = 0x006380d8u;
    r->ecx = 0u;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->edi = c.edi;
    // dec ecx 1 -> 0: ZF = PF = 1, SF = OF = AF = 0; CF = 0 from the equal "cmp ebx, 0x638cd8"
    r->eflags = (r->eflags & ~0x8d5u) | 0x044u;
}
GP4_IMPL(0x0041c31e, FUN_0041c31e, "regs(esi:ptr[0x3b0]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=curves_0062ecb0+dw_0041c027+state_0041bce9+state_0041b950+state_0041bd56+state_0041bbef+state_0041bd28+div_0041bc3a+state_0041befb+state_0041c145")
