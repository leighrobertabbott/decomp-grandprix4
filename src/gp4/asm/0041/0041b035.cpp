#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_0041b035 {
    uint8_t Unknown00[0x202];
    uint8_t Slot202;
};

// Integrates 3 pairs of qwords (position at 0x006020b0 + slot*0x60 + 0..16, velocity at
// 0x006020e0 + slot*0x60 + 0..16) for [0x60384c] / ([0x60384c] / [0x6028f0]) sub-steps.
// Everything between the loads and the stores stays in 80-bit x87 registers in the original.
// The three coefficient dwords at 0x006028f4 / 0x00602900 / 0x00623ea0 are outside the 22-slot
// table (0x006020b0..0x006028f0), so they do not change during the loop.
// FUNCTION: GP4 0x0041b035
void __cdecl FUN_0041b035(gp4::Regs* r) {
    using namespace gp4::x87;

    const Unknown_0041b035* body = reinterpret_cast<const Unknown_0041b035*>(r->esi);
    const uint32_t offset = static_cast<uint32_t>(body->Slot202) * 0x60u;

    int32_t rem;
    int32_t steps = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x0060384c)),
                                GP4_GLOBAL(int32_t, 0x006028f0), &rem);
    if (steps < 1) {
        steps = 1;
    }
    const int32_t count = steps;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x0060384c)),
                                         count, &rem);
    GP4_GLOBAL(int32_t, 0x006f2bf8) = quotient;

    const Ext scale = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf8)) * ext(GP4_GLOBAL(double, 0x006f2a1c));
    const Ext half = ext(GP4_GLOBAL(double, 0x006f2a54));
    const Ext c2 = ext(GP4_GLOBAL(double, 0x006f2a24));

    Ext dragK[3];
    Ext velK[3];
    Ext constK[3];
    for (uint32_t j = 0; j != 3; ++j) {
        dragK[j] = ext_i32(*reinterpret_cast<int32_t*>(0x006028f4u + j * 4u)) * c2;
        velK[j] = ext_i32(*reinterpret_cast<int32_t*>(0x00602900u + j * 4u)) * c2;
        constK[j] = ext_i32(*reinterpret_cast<int32_t*>(0x00623ea0u + j * 4u));
    }

    for (int32_t n = count; n != 0; --n) {
        for (uint32_t j = 0; j != 3; ++j) {
            double* vel = reinterpret_cast<double*>(0x006020e0u + offset + j * 8u);
            double* pos = reinterpret_cast<double*>(0x006020b0u + offset + j * 8u);

            const Ext v = ext(*vel);
            const Ext p = ext(*pos);
            const Ext force = (((fabs(p) * p) * dragK[j]) + (velK[j] * v)) + constK[j];
            const Ext vNew = fchs(force) * scale + v;
            *vel = narrow(vNew);
            *pos = narrow((v + vNew) * half * scale + p);
        }
    }

    r->eax = static_cast<uint32_t>(quotient);
    r->ebx = static_cast<uint32_t>(quotient);
    r->ecx = 0u;
    r->edx = static_cast<uint32_t>(rem);
    // last flag writer is "dec ecx" (1 -> 0); CF still comes from "cmp esi, 0xc" (equal): CF = 0
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041b035, FUN_0041b035, "regs(esi:ptr[0x260]:layout=rec_00418c68) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041b035")
