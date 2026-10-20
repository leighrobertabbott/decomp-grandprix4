#include <gp4/gp4.h>

// Per-part tables of the 23-part mass model (stride 0x10).
struct PartInertia {      // 0x62a424: static per-unit-mass inertia terms and mass
    int32_t Ixx;
    int32_t Iyy;
    int32_t Izz;
    int32_t Mass;
};
struct PartPosition {     // 0x62a5ac: static part position
    int32_t X;
    int32_t Y;
    int32_t Z;
    int32_t Unknown0C;
};
struct PartWeighted {     // 0x62a7cc: mass-weighted position and mass
    int32_t MassX;
    int32_t MassY;
    int32_t MassZ;
    int32_t Mass;
};

static const int kPartCount = 23;

// Over the parts selected by the 23-bit mask in eax: centroid = sum(m*p) / sum(m) (sum clamped to 1),
// then the parallel-axis sums ((dy^2 + dz^2 + Ixx) * m, ...) as 64-bit totals divided by the mass sum.
// pushal/popal: every register is preserved; the flags are those of the final loop compare.
// FUNCTION: GP4 0x00444517
void __cdecl FUN_00444517(gp4::Regs* r) {
    const uint32_t mask = r->eax;
    const PartInertia* inertia = GP4_ARRAY(const PartInertia, 0x0062a424);
    const PartPosition* position = GP4_ARRAY(const PartPosition, 0x0062a5ac);
    const PartWeighted* weighted = GP4_ARRAY(const PartWeighted, 0x0062a7cc);

    uint32_t sumX = 0, sumY = 0, sumZ = 0, sumMass = 0;
    uint32_t bits = mask;
    for (int i = 0; i < kPartCount; ++i, bits >>= 1) {
        if (bits & 1u) {
            sumX += (uint32_t)weighted[i].MassX;
            sumY += (uint32_t)weighted[i].MassY;
            sumZ += (uint32_t)weighted[i].MassZ;
            sumMass += (uint32_t)weighted[i].Mass;
        }
    }
    if (sumMass == 0) sumMass = 1;
    const int32_t cx = (int32_t)sumX / (int32_t)sumMass;
    const int32_t cy = (int32_t)sumY / (int32_t)sumMass;
    const int32_t cz = (int32_t)sumZ / (int32_t)sumMass;
    GP4_GLOBAL(int32_t, 0x0062a064) = cx;
    GP4_GLOBAL(int32_t, 0x0062a068) = cy;
    GP4_GLOBAL(int32_t, 0x0062a06c) = cz;

    int64_t accX = 0, accY = 0, accZ = 0;
    uint32_t mass = 0;
    bits = mask;
    for (int i = 0; i < kPartCount; ++i, bits >>= 1) {
        if (bits & 1u) {
            const uint32_t dx = (uint32_t)cx - (uint32_t)position[i].X;
            const uint32_t dy = (uint32_t)cy - (uint32_t)position[i].Y;
            const uint32_t dz = (uint32_t)cz - (uint32_t)position[i].Z;
            const int32_t termX = (int32_t)(dy * dy + dz * dz + (uint32_t)inertia[i].Ixx);
            accX += (int64_t)termX * (int64_t)inertia[i].Mass;
            const int32_t termY = (int32_t)(dx * dx + dz * dz + (uint32_t)inertia[i].Iyy);
            accY += (int64_t)termY * (int64_t)inertia[i].Mass;
            const int32_t termZ = (int32_t)(dx * dx + dy * dy + (uint32_t)inertia[i].Izz);
            accZ += (int64_t)termZ * (int64_t)inertia[i].Mass;
            mass += (uint32_t)weighted[i].Mass;
        }
    }
    GP4_GLOBAL(int64_t, 0x0062a594) = accX;
    GP4_GLOBAL(int64_t, 0x0062a59c) = accY;
    GP4_GLOBAL(int64_t, 0x0062a5a4) = accZ;

    int32_t rem;
    GP4_GLOBAL(int32_t, 0x0062a070) = gp4::idiv64(accX, (int32_t)mass, &rem);
    GP4_GLOBAL(int32_t, 0x0062a074) = gp4::idiv64(accY, (int32_t)mass, &rem);
    GP4_GLOBAL(int32_t, 0x0062a078) = gp4::idiv64(accZ, (int32_t)mass, &rem);
    GP4_GLOBAL(uint32_t, 0x0062a07c) = mass;

    // Last flag writer: cmp esi,0x170 with esi == 0x170 (ZF = PF = 1; CF = AF = SF = OF = 0).
    r->eflags = (r->eflags & ~0x8d5u) | 0x044u;
}
GP4_IMPL(0x00444517, FUN_00444517, "regs(eax:u32[1..0x7fffff]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00444517")
