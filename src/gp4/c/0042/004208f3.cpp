#include <gp4/gp4.h>

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x004208f3
void __cdecl FUN_004208f3(gp4::Regs* r) {
    uint8_t* state = GP4_ARRAY(uint8_t, 0x006380d8);
    int32_t* ring0 = GP4_ARRAY(int32_t, 0x00622940);
    int32_t* ring1 = GP4_ARRAY(int32_t, 0x006229c0);
    int32_t* ring2 = GP4_ARRAY(int32_t, 0x00622a40);
    int32_t* ring3 = GP4_ARRAY(int32_t, 0x00622ac0);
    int32_t index = GP4_GLOBAL(int32_t, 0x0062291c);

    // Push the four sampled values into the 32-slot rings and keep running sums.
    int32_t v0 = GP4_FIELD(int32_t, state, 0xc8);
    ring0[index] = v0;
    GP4_GLOBAL(int32_t, 0x00622930) += v0;
    int32_t v1 = GP4_FIELD(int32_t, state, 0x3c8);
    ring1[index] = v1;
    GP4_GLOBAL(int32_t, 0x00622934) += v1;
    int32_t v2 = GP4_FIELD(int32_t, state, 0x6c8);
    ring2[index] = v2;
    GP4_GLOBAL(int32_t, 0x00622938) += v2;
    int32_t v3 = GP4_FIELD(int32_t, state, 0x9c8);
    ring3[index] = v3;
    GP4_GLOBAL(int32_t, 0x0062293c) += v3;

    GP4_GLOBAL(int32_t, 0x00622918) += 1;
    int32_t last = GP4_GLOBAL(int32_t, 0x00622918);
    const int32_t window = GP4_GLOBAL(int32_t, 0x006228b0);
    if (last > window) {
        // Drop the sample that has just left the window.
        int32_t old = index - window;
        if (old < 0) old += 0x20;
        GP4_GLOBAL(int32_t, 0x00622930) -= ring0[old];
        GP4_GLOBAL(int32_t, 0x00622934) -= ring1[old];
        GP4_GLOBAL(int32_t, 0x00622938) -= ring2[old];
        last = ring3[old];
        GP4_GLOBAL(int32_t, 0x0062293c) -= last;
        r->edx = static_cast<uint32_t>(old);
    }

    uint32_t next = static_cast<uint32_t>(index) + 1u;
    const uint32_t diff = next - 0x20u;
    const uint32_t flags = SubtractFlags(next, 0x20u, diff);
    if (static_cast<int32_t>(next) >= 0x20) next = diff;
    GP4_GLOBAL(int32_t, 0x0062291c) = static_cast<int32_t>(next);

    r->eax = static_cast<uint32_t>(last);
    r->ebx = 0x006380d8u;
    r->ecx = next;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004208f3, FUN_004208f3, "regs(edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=recs_006380d8")
