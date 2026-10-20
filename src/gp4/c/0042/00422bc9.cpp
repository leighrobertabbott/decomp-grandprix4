#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parity8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parity8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The fixed body at 0x00622e98 (edx in the original): its matrix qwords +0x58.. are read relative to it.
static __declspec(noinline) uint8_t* viewBody() {
    return GP4_ARRAY(uint8_t, 0x00622e98);
}

// Contact-point emission: the count [0x006082b4] becomes [0x006097cc], the remaining budget
// 0x45 - count goes to [0x006082bc] and the points per record max(4 - count, 1) to [0x006082b8].
// The body's matrix row (+0x58/+0x60/+0x68) is turned into angles by 0x00402305; its elevation
// (x 2^31/pi, rounded) gives the sign flag [0x006082d8] and the angle 0x40000000 - |a| (folded at a
// quarter turn) in [0x006082d4]. The heading atan2(+0x90, +0x78) and the projected length
// (cos * +0x78 + sin * +0x90) * 16384 give [0x006082dc] (+ a half turn for a non-negative length)
// before 0x00422ac2 and its offset [0x006080c4]. Then each record of the list [0x006097ac]
// (stride 0x14) with a nonzero kind +0xc picks its terms from the tables 0x006082f4 / 0x006082e8 and
// emits its points into 0x00620fc0 (esi counts them) through 0x004229b2, 0x004229f5 or 0x00422a65 by
// comparing [0x006082d4] with [0x006082e0] / [0x006082e4], until 0x15 points, the budget or the
// count is reached; the emitted count is added to [0x006097cc].
// FUNCTION: GP4 0x00422bc9
void __cdecl FUN_00422bc9(gp4::Regs* r) {
    using namespace gp4::x87;

    GP4_GLOBAL(uint32_t, 0x006080bc) = 0x4000u;
    GP4_GLOBAL(uint32_t, 0x006080c4) = 0u;
    GP4_GLOBAL(uint32_t, 0x006082bc) = 0x45u - GP4_GLOBAL(uint32_t, 0x006082b4);
    GP4_GLOBAL(uint32_t, 0x006097cc) = GP4_GLOBAL(uint32_t, 0x006082b4);
    int32_t perRecord = static_cast<int32_t>(4u - GP4_GLOBAL(uint32_t, 0x006097cc));
    if (perRecord < 1) {
        perRecord = 1;
    }
    GP4_GLOBAL(int32_t, 0x006082b8) = perRecord;

    uint8_t* body = viewBody();
    GP4_GLOBAL(double, 0x00622c80) = GP4_FIELD(double, body, 0x58);
    GP4_GLOBAL(double, 0x00622c88) = GP4_FIELD(double, body, 0x60);
    GP4_GLOBAL(double, 0x00622c90) = GP4_FIELD(double, body, 0x68);
    gp4::cdecl_<0x00402305, void>();

    const int32_t elevation = fistp32(ext(GP4_GLOBAL(double, 0x00622c98)) * ext(GP4_GLOBAL(double, 0x006f2be4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = elevation;
    GP4_GLOBAL(uint32_t, 0x006082d8) = 0u;
    uint32_t folded = static_cast<uint32_t>(elevation);
    if (elevation < 0) {
        GP4_GLOBAL(uint32_t, 0x006082d8) = 0x80000000u;
        folded = 0u - folded;
    }
    if (folded > 0x40000000u) {
        folded = 0x80000000u - folded;
    }
    GP4_GLOBAL(uint32_t, 0x006082d4) = 0x40000000u - folded;

    const Ext heading = fpatan(ext(GP4_FIELD(double, body, 0x90)), ext(GP4_FIELD(double, body, 0x78)));
    Ext sine, cosine;
    fsincos(heading, &sine, &cosine);
    const Ext length = cosine * ext(GP4_FIELD(double, body, 0x78)) + sine * ext(GP4_FIELD(double, body, 0x90));
    const int32_t projected = fistp32(length * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = projected;
    GP4_GLOBAL(int32_t, 0x006082dc) = fistp32(heading * ext(GP4_GLOBAL(double, 0x006f2be4)));
    if (projected >= 0) {
        GP4_GLOBAL(uint32_t, 0x006082dc) += 0x80000000u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = static_cast<uint32_t>(projected);
    c.edx = reinterpret_cast<uint32_t>(body);
    gp4::call_regs(0x00422ac2, c);

    c.eax = GP4_GLOBAL(uint32_t, 0x006080c4);
    GP4_GLOBAL(uint32_t, 0x006082dc) += c.eax;
    c.edi = 0x00620fc0u;
    c.esi = 0u;
    c.ecx = 0u;
    c.ebx = GP4_GLOBAL(uint32_t, 0x006097ac);
    for (;;) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
        const uint32_t kind = GP4_FIELD(uint32_t, record, 0xc);
        if (kind != 0u) {
            c.edx = kind;
            GP4_GLOBAL(uint32_t, 0x006082c8) = GP4_ARRAY(uint32_t, 0x006082f4)[kind];
            uint32_t lateral = GP4_ARRAY(uint32_t, 0x006082e8)[kind];
            if (GP4_GLOBAL(int32_t, 0x006082d8) >= 0) {
                lateral = 0u - lateral;
            }
            GP4_GLOBAL(uint32_t, 0x006082c4) = lateral;
            GP4_GLOBAL(uint32_t, 0x006082c0) = 0u;
            uint32_t roll = GP4_GLOBAL(uint32_t, 0x00607f54);
            if (static_cast<int32_t>(roll) < 0) {
                roll = 0u - roll;
            }
            if (roll > 0x40000000u) {
                GP4_GLOBAL(uint32_t, 0x006082c0) = 0x80000000u;
            }
            c.eax = GP4_GLOBAL(uint32_t, 0x006082d4);
            if (static_cast<int32_t>(c.eax) >= GP4_GLOBAL(int32_t, 0x006082e0)) {
                gp4::call_regs(0x004229b2, c);
            } else if (static_cast<int32_t>(c.eax) >= GP4_GLOBAL(int32_t, 0x006082e4)) {
                gp4::call_regs(0x004229f5, c);
            } else {
                if (GP4_GLOBAL(int32_t, 0x006082b8) < 3) {
                    GP4_GLOBAL(int32_t, 0x006082b8) = 3;
                }
                gp4::call_regs(0x00422a65, c);
            }
        }
        if (static_cast<int32_t>(c.esi) >= 0x15) {
            break;
        }
        if (static_cast<int32_t>(c.esi) >= GP4_GLOBAL(int32_t, 0x006082bc)) {
            break;
        }
        c.ebx += 0x14u;
        c.ecx += 1u;
        if (c.ecx == GP4_GLOBAL(uint32_t, 0x006097cc)) {
            break;
        }
    }

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x006097cc);
    GP4_GLOBAL(uint32_t, 0x006097cc) = before + c.esi;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(before, c.esi);
}
GP4_IMPL(0x00422bc9, FUN_00422bc9, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=view_00609860+state_0042430b+state_00422ac2+state_00424011+state_00422bc9")
