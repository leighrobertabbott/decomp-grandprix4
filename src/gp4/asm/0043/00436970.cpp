#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) >> 20) & 0x800u);
}

static inline uint32_t& g32(uint32_t addr) { return GP4_GLOBAL(uint32_t, addr); }
static inline uint16_t& g16(uint32_t addr) { return GP4_GLOBAL(uint16_t, addr); }

// imul [0x603854]; shrd eax, edx, 24
static inline uint32_t scaleByRate(uint32_t value) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value))
        * static_cast<int64_t>(static_cast<int32_t>(g32(0x00603854)));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
}

static inline uint32_t sar32(uint32_t value) {
    return static_cast<uint32_t>(static_cast<int32_t>(value) >> 1);
}

static inline uint16_t sar16(uint16_t value) {
    return static_cast<uint16_t>(static_cast<int16_t>(value) >> 1);
}

static inline gp4::Regs scratchRegs(const gp4::Regs& state) {
    gp4::Regs t = state;
    t.st_in = 0;
    t.st_out = 0;
    t.st_xin_mask = 0;
    t.st_xout_mask = 0;
    return t;
}

// 0x00436857: carry = the two collision objects at 0x629e0c / 0x629e34 overlap (registers kept).
static inline bool collides(const gp4::Regs& state) {
    gp4::Regs t = scratchRegs(state);
    gp4::call_regs(0x00436857, t);
    return (t.eflags & 1u) != 0u;
}

// Halves the per-step deltas (heading words, the four position deltas) and the step weight.
static inline void halveSteps() {
    g16(0x00629e16) = sar16(g16(0x00629e16));
    g16(0x00629e3e) = sar16(g16(0x00629e3e));
    g32(0x00934de0) = sar32(g32(0x00934de0));
    g32(0x00934de4) = sar32(g32(0x00934de4));
    g32(0x00934de8) = sar32(g32(0x00934de8));
    g32(0x00934dec) = sar32(g32(0x00934dec));
    g32(0x00934df8) = g32(0x00934df8) >> 1;
}

// One step back in time: positions minus the deltas, headings minus their deltas, time plus the weight.
static inline void stepBack(gp4::Regs& cpu) {
    g32(0x00629e0c) -= g32(0x00934de0);
    g32(0x00629e10) -= g32(0x00934de4);
    g32(0x00629e34) -= g32(0x00934de8);
    g32(0x00629e38) -= g32(0x00934dec);
    g16(0x00629e14) = static_cast<uint16_t>(g16(0x00629e14) - g16(0x00629e16));
    g16(0x00629e3c) = static_cast<uint16_t>(g16(0x00629e3c) - g16(0x00629e3e));
    cpu.eax = g32(0x00934df8);
    g32(0x00629e68) += cpu.eax;
}

static inline void stepForward(gp4::Regs& cpu) {
    g32(0x00629e0c) += g32(0x00934de0);
    g32(0x00629e10) += g32(0x00934de4);
    g32(0x00629e34) += g32(0x00934de8);
    g32(0x00629e38) += g32(0x00934dec);
    g16(0x00629e14) = static_cast<uint16_t>(g16(0x00629e14) + g16(0x00629e16));
    g16(0x00629e3c) = static_cast<uint16_t>(g16(0x00629e3c) + g16(0x00629e3e));
    cpu.eax = g32(0x00934df8);
    g32(0x00629e68) -= cpu.eax;
}

static inline void finish(gp4::Regs* r, const gp4::Regs& cpu, uint32_t flags) {
    r->eax = cpu.eax;
    r->ebx = cpu.ebx;
    r->ecx = cpu.ecx;
    r->edx = cpu.edx;
    r->esi = cpu.esi;
    r->edi = cpu.edi;
    r->ebp = cpu.ebp;
    r->eflags = (cpu.eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x00436970
// Swept collision of two cars (esi, ebx). Their positions (+0/+4) and headings (+0xe) are copied to the
// collision objects 0x629e0c / 0x629e34, the per-step deltas are the velocities +0xa4/+0xa8 scaled by
// the rate [0x603854] >> 24 (heading deltas +0xe - +0xec), halved until the largest |delta| is below
// 0x8000 (the weight 0x4000 halves with them). Stepping back in time (time [0x629e68] += weight) while
// the objects do not collide, at most [0x629e64] steps: no collision returns clc. On a collision it
// steps back until they are apart (or the count runs out: then 0x0043687a separates them along the
// line of centres), refines over 4 halvings, publishes the contact words for the player car
// [0x679d30], writes the positions/headings back and returns stc.
void __cdecl FUN_00436970(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    gp4::Regs cpu = scratchRegs(*r);

    g32(0x00629e1c) = 0;
    g32(0x00629e24) = 0;
    g32(0x00629e30) = 0;
    g32(0x00629e44) = 0;
    g32(0x00629e4c) = 0;
    g32(0x00629e58) = 0;
    g32(0x00934df4) = 0;
    g32(0x00629e0c) = GP4_FIELD(uint32_t, first, 0);
    g32(0x00629e10) = GP4_FIELD(uint32_t, first, 4);
    g32(0x00629e34) = GP4_FIELD(uint32_t, second, 0);
    g32(0x00629e38) = GP4_FIELD(uint32_t, second, 4);

    const uint16_t headingA = GP4_FIELD(uint16_t, first, 0xe);
    g16(0x00629e14) = headingA;
    g16(0x00629e16) = static_cast<uint16_t>(headingA - GP4_FIELD(uint16_t, first, 0xec));
    const uint16_t headingB = GP4_FIELD(uint16_t, second, 0xe);
    g16(0x00629e3c) = headingB;
    g16(0x00629e3e) = static_cast<uint16_t>(headingB - GP4_FIELD(uint16_t, second, 0xec));

    g32(0x00934de0) = scaleByRate(GP4_FIELD(uint32_t, first, 0xa4));
    g32(0x00934de4) = scaleByRate(GP4_FIELD(uint32_t, first, 0xa8));
    g32(0x00934de8) = scaleByRate(GP4_FIELD(uint32_t, second, 0xa4));
    g32(0x00934dec) = scaleByRate(GP4_FIELD(uint32_t, second, 0xa8));

    uint32_t spanX = g32(0x00934de8) - g32(0x00934de0);
    if (static_cast<int32_t>(spanX) < 0) spanX = 0u - spanX;
    uint32_t spanY = g32(0x00934dec) - g32(0x00934de4);
    if (static_cast<int32_t>(spanY) < 0) spanY = 0u - spanY;
    cpu.edx = spanY;
    g32(0x00934df4) = static_cast<int32_t>(spanX) < static_cast<int32_t>(spanY) ? spanY : spanX;
    g32(0x00934dfc) = 1;
    g32(0x00934df8) = 0x4000;
    while (static_cast<int32_t>(g32(0x00934df4)) >= 0x8000) {
        halveSteps();
        g32(0x00934df4) = g32(0x00934df4) >> 1;
        g32(0x00934dfc) = g32(0x00934dfc) << 1;
    }

    cpu.eax = g32(0x00629e64);
    g32(0x00934dfc) = cpu.eax;
    g32(0x00934dfc) = 0;
    g32(0x00629e68) = 0;

    // Step back until the objects collide or the step budget is used up.
    while (!collides(cpu)) {
        cpu.eax = g32(0x00934dfc);
        if (cpu.eax == g32(0x00629e64)) {
            finish(r, cpu, 0x44u);  // cmp equal, then clc
            return;
        }
        g32(0x00934dfc) += 1;
        stepBack(cpu);
    }

    // Colliding: keep stepping back while they still overlap.
    bool separate = false;
    for (;;) {
        cpu.eax = g32(0x00934dfc);
        if (cpu.eax == g32(0x00629e64)) {
            separate = true;
            break;
        }
        g32(0x00934dfc) += 1;
        stepBack(cpu);
        if (!collides(cpu)) break;
    }

    if (separate) {
        gp4::call_regs(0x0043687a, cpu);
        do {
            g32(0x00629e0c) -= g32(0x00934de0);
            g32(0x00629e10) -= g32(0x00934de4);
            g32(0x00629e34) -= g32(0x00934de8);
            cpu.eax = g32(0x00934dec);
            g32(0x00629e38) -= cpu.eax;
        } while (collides(cpu));
    } else {
        // Binary refinement over four halvings: forward while apart, back while overlapping.
        g32(0x00934dfc) = 0;
        bool forward = true;
        for (;;) {
            halveSteps();
            if (forward) {
                stepForward(cpu);
            } else {
                stepBack(cpu);
            }
            g32(0x00934dfc) += 1;
            const bool hit = collides(cpu);
            if (static_cast<int32_t>(g32(0x00934dfc)) >= 4) break;
            forward = !hit;
        }
    }

    const uint32_t esiValue = r->esi;
    const uint32_t ebxValue = r->ebx;
    uint32_t flags;
    bool player;
    if (esiValue == g32(0x00679d30)) {
        flags = cmpFlags32(esiValue, g32(0x00679d30));
        player = true;
    } else {
        flags = cmpFlags32(ebxValue, g32(0x00679d30));
        player = ebxValue == g32(0x00679d30);
    }
    if (player) {
        g32(0x00624728) = g32(0x00629e1c);
        g32(0x0062472c) = g32(0x00629e44);
        g32(0x00624730) = g32(0x00629e24);
        g32(0x00624734) = g32(0x00629e4c);
        g32(0x00624738) = g32(0x00629e30);
        g32(0x0062473c) = g32(0x00629e58);
        g32(0x00624238) = esiValue;
        g32(0x0062423c) = ebxValue;
    }
    GP4_FIELD(uint32_t, first, 0) = g32(0x00629e0c);
    GP4_FIELD(uint32_t, first, 4) = g32(0x00629e10);
    GP4_FIELD(uint32_t, second, 0) = g32(0x00629e34);
    GP4_FIELD(uint32_t, second, 4) = g32(0x00629e38);
    GP4_FIELD(uint16_t, first, 0xe) = g16(0x00629e14);
    GP4_FIELD(uint16_t, second, 0xe) = g16(0x00629e3c);
    finish(r, cpu, (flags & ~1u) | 1u);  // stc
}
GP4_IMPL(0x00436970, FUN_00436970, "regs(esi:ptr[0x420]:layout=car_0042ba87+obj_0043659c, ebx:ptr[0x420]:layout=car_0042ba87+obj_0043659c) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00418756+pair_00436970")
