#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_0041b0ed {
    uint8_t Unknown00[0xcd];
    uint8_t FlagsCD;
    uint8_t UnknownCE[0x202 - 0xce];
    uint8_t Slot202;
};

static inline int32_t absolute(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
}

// FUNCTION: GP4 0x0041b0ed
// Slot = byte esi+0x202 (*0x60 into the f64 table at 0x006020b0). With bit 7 of [esi+0xcd] clear:
// once |[0x00638038]| >> 16 reaches [0x00602954] the bit is set and the slot's qword +0x28 gets
// -(0x0041b286([0x00638038] >> 16) << 8), qword +0x58 the dword [0x00602fd4]. With the bit set it
// is cleared again when max(|[0x00638038]| >> 16, |round(+0x28)| >> 8) < [0x00602958] and
// max(|[0x00602fd4]|, |round(+0x58)|) < [0x0060295c]. Then the two axis pairs (position +0x18 /
// +0x28, velocity +0x48 / +0x58) are integrated for N = max(1, [0x0060384c] / [0x006028f0])
// sub-steps of dt = ([0x0060384c] / N) * qword [0x006f2a1c]: the force from the dword coefficients
// at 0x00602910 / 0x0060291c / 0x00623eac (stride 8, times qword 0x006f2a24 / 0x006f2a24 /
// 0x006f2ad4) gives v' = v - force * dt and p' = p + (v + v') * [0x006f2a54] * dt, all 80-bit in
// the original until each qword store. eax = ebx = [0x006f2bf8] = the step, edx = the remainder,
// ecx = 0, ebp = 0x0041b286's when it ran; the flags are those of the final "dec ecx".
void __cdecl FUN_0041b0ed(gp4::Regs* r) {
    using namespace gp4::x87;

    Unknown_0041b0ed* body = reinterpret_cast<Unknown_0041b0ed*>(static_cast<uintptr_t>(r->esi));
    const uint32_t offset = static_cast<uint32_t>(body->Slot202) * 0x60u;
    uint8_t* slot = GP4_ARRAY(uint8_t, 0x006020b0) + offset;

    if ((body->FlagsCD & 0x80u) == 0u) {
        const int32_t level = absolute(GP4_GLOBAL(int32_t, 0x00638038)) >> 16;
        if (level >= GP4_GLOBAL(int32_t, 0x00602954)) {
            body->FlagsCD = static_cast<uint8_t>(body->FlagsCD | 0x80u);
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00638038) >> 16);
            gp4::call_regs(0x0041b286, call);
            r->ebp = call.ebp;
            GP4_GLOBAL(uint32_t, 0x006f2bf4) = 0u - (call.eax << 8);
            GP4_FIELD(double, slot, 0x28) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006f2bf4));
            GP4_FIELD(double, slot, 0x58) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00602fd4));
        }
    } else {
        int32_t level = absolute(GP4_GLOBAL(int32_t, 0x00638038)) >> 16;
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_FIELD(double, slot, 0x28)));
        const int32_t side = absolute(GP4_GLOBAL(int32_t, 0x006f2bf4)) >> 8;
        if (level < side) {
            level = side;
        }
        if (level < GP4_GLOBAL(int32_t, 0x00602958)) {
            int32_t load = absolute(GP4_GLOBAL(int32_t, 0x00602fd4));
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_FIELD(double, slot, 0x58)));
            const int32_t other = absolute(GP4_GLOBAL(int32_t, 0x006f2bf4));
            if (load < other) {
                load = other;
            }
            if (load < GP4_GLOBAL(int32_t, 0x0060295c)) {
                body->FlagsCD = static_cast<uint8_t>(body->FlagsCD & 0x7fu);
            }
        }
    }

    int32_t rem;
    int32_t steps = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x0060384c)),
                                GP4_GLOBAL(int32_t, 0x006028f0), &rem);
    if (steps < 1) {
        steps = 1;
    }
    const int32_t step = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x0060384c)), steps, &rem);
    GP4_GLOBAL(int32_t, 0x006f2bf8) = step;

    const Ext dt = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf8)) * ext(GP4_GLOBAL(double, 0x006f2a1c));
    for (int32_t n = steps; n != 0; --n) {
        for (uint32_t k = 0; k != 0x10u; k += 8u) {
            double* vel = reinterpret_cast<double*>(slot + 0x48u + k * 2u);
            double* pos = reinterpret_cast<double*>(slot + 0x18u + k * 2u);
            const Ext v = ext(*vel);
            const Ext p = ext(*pos);
            const Ext spring = (fabs(p) * p)
                * (ext_i32(*reinterpret_cast<int32_t*>(0x00602910u + k)) * ext(GP4_GLOBAL(double, 0x006f2a24)));
            const Ext damping = (ext_i32(*reinterpret_cast<int32_t*>(0x0060291cu + k)) * ext(GP4_GLOBAL(double, 0x006f2a24))) * v;
            const Ext bias = ext_i32(*reinterpret_cast<int32_t*>(0x00623eacu + k)) * ext(GP4_GLOBAL(double, 0x006f2ad4));
            const Ext force = (spring + damping) + bias;
            const Ext vNew = fchs(force) * dt + v;
            *vel = narrow(vNew);
            const Ext shift = ((v + vNew) * ext(GP4_GLOBAL(double, 0x006f2a54))) * dt;
            *pos = narrow(shift + ext(*pos));
        }
    }

    r->eax = static_cast<uint32_t>(step);
    r->ebx = static_cast<uint32_t>(step);
    r->ecx = 0u;
    r->edx = static_cast<uint32_t>(rem);
    // last flag writer is "dec ecx" (1 -> 0); CF still comes from "cmp esi, 0x10" (equal): CF = 0
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041b0ed, FUN_0041b0ed, "regs(ebp:u32, esi:ptr[0x420]:layout=car_0042ba87+rec_00418c68) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b035")
