#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test byte, 0xff` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Records addressed through esi and edi (only the fields this routine reads).
struct Unknown_00433689_Esi {
    uint8_t Unknown00[0x0e];
    uint16_t Unknown0E;
    uint8_t Unknown10[0x71 - 0x10];
    uint8_t Flags71;
};

struct Unknown_00433689_Edi {
    uint16_t Unknown00;
    uint8_t Unknown02[0xe4 - 0x02];
    uint32_t UnknownE4;
};

// Two byte flags are set from the 16-bit difference of the words at esi+0xe and edi+0:
// DAT_00601baf = 0xff when its magnitude is >= 0x4000; DAT_00601bae = 0xff when that is not equal
// to bit 2 of byte esi+0x71. The sign of byte DAT_00868582 then picks 0x40cf62 (eax = 0x7d) or
// 0x40cf78 (eax = -0x7d, the result in DAT_00601bf8 negated); ecx = DAT_00601bf8 (negated if
// DAT_00601bae is set) plus the dword at edi+0xe4. eax / edx come from the callee and the flags are
// those of the final add.
// FUNCTION: GP4 0x00433689
void __cdecl FUN_00433689(gp4::Regs* r) {
    const Unknown_00433689_Esi* self = reinterpret_cast<const Unknown_00433689_Esi*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00433689_Edi* other = reinterpret_cast<const Unknown_00433689_Edi*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint8_t, 0x00601bae) = 0u;
    GP4_GLOBAL(uint8_t, 0x00601baf) = 0u;

    uint32_t difference = (static_cast<uint32_t>(self->Unknown0E) - static_cast<uint32_t>(other->Unknown00)) & 0xffffu;
    if ((difference & 0x8000u) != 0u) {
        difference = (0u - difference) & 0xffffu;
    }
    const bool large = difference >= 0x4000u;
    const bool bit2 = (self->Flags71 & 4u) != 0u;
    if (large) {
        GP4_GLOBAL(uint8_t, 0x00601baf) = 0xffu;
    }
    if (large != bit2) {
        GP4_GLOBAL(uint8_t, 0x00601bae) = 0xffu;
    }

    const uint32_t selector = GP4_GLOBAL(uint8_t, 0x00868582);
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eflags = (r->eflags & ~0x8d5u) | testFlags8(selector);

    uint32_t ecx;
    if ((selector & 0x80u) != 0u) {
        callee.eax = 0x7du;
        gp4::call_regs(0x0040cf62, callee);
        ecx = GP4_GLOBAL(uint32_t, 0x00601bf8);
    } else {
        callee.eax = 0xffffff83u;
        gp4::call_regs(0x0040cf78, callee);
        ecx = 0u - GP4_GLOBAL(uint32_t, 0x00601bf8);
    }
    if (GP4_GLOBAL(uint8_t, 0x00601bae) != 0u) {
        ecx = 0u - ecx;
    }

    const uint32_t addend = other->UnknownE4;
    r->eax = callee.eax;
    r->edx = callee.edx;
    r->ecx = ecx + addend;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(ecx, addend);
}
GP4_IMPL(0x00433689, FUN_00433689, "regs(esi:ptr[0x79]:bytes, edi:ptr[0xec]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
