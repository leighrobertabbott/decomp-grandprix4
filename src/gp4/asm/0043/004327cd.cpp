#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Car record fields used here.
struct Unknown_004327cd {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint8_t Unknown62[0x19];
    uint8_t Flags7B;
    uint8_t Unknown7C[6];
    uint8_t Flags82;
    uint8_t Unknown83[0x7b];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x63];
    uint8_t Flags162;
    uint8_t Unknown163[0x7d];
    uint16_t Unknown1E0;
    uint8_t Unknown1E2[0x92];
    uint32_t Unknown274;
};

// FUNCTION: GP4 0x004327cd
// Initialises the car record at esi: +0x162 |= 1, +0x7b |= 0x40, +0xfe &= ~0x20, +0x82 |= 1,
// word +0x1e0 = word +0x60, dword +0x274 = [0x0066d814]. Then, when the sign bit of byte
// [0x007c2ea9] is clear, runs 0x0043260e; otherwise sets +0x7b |= 8 and, for the car held in
// [0x00679d2c], also copies [0x00679b18] to [0x0066d854], clears [0x0066d848] and copies the word
// [0x0088f6a0] to the word [0x0066d884]. pushal/popal preserve every GPR; the flags are those of
// the last flag writer: 0x0043260e, or "cmp esi, [0x00679d2c]".
void __cdecl FUN_004327cd(gp4::Regs* r) {
    Unknown_004327cd* car = reinterpret_cast<Unknown_004327cd*>(static_cast<uintptr_t>(r->esi));

    car->Flags162 |= 1u;
    car->Flags7B |= 0x40u;
    car->FlagsFE &= 0xdfu;
    car->Flags82 |= 1u;
    car->Unknown1E0 = car->Unknown60;
    const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
    car->Unknown274 = now;

    const uint32_t selector = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags;
    if ((selector & 0x80u) == 0u) {
        // "test byte [0x7c2ea9], 0xff" with the sign clear: CF = OF = AF = 0, ZF/PF from the byte
        gp4::Regs call = *r;
        call.eax = now;
        call.eflags = (r->eflags & ~0x8d5u) | parityFlag(selector) | (selector == 0u ? 0x40u : 0u);
        gp4::call_regs(0x0043260e, call);
        flags = call.eflags & 0x8d5u;
    } else {
        car->Flags7B |= 8u;
        const uint32_t current = GP4_GLOBAL(uint32_t, 0x00679d2c);
        if (r->esi == current) {
            GP4_GLOBAL(uint32_t, 0x0066d854) = GP4_GLOBAL(uint32_t, 0x00679b18);
            GP4_GLOBAL(uint32_t, 0x0066d848) = 0u;
            GP4_GLOBAL(uint16_t, 0x0066d884) = GP4_GLOBAL(uint16_t, 0x0088f6a0);
        }
        flags = subFlags(r->esi, current);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004327cd, FUN_004327cd, "regs(esi:ptr[0x27c]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_0043260e+state_004327cd")
