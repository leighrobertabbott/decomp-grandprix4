#include <gp4/gp4.h>

struct Unknown_00432a0f_Car {
    uint8_t Unknown00[0x70];
    uint8_t Flag70;
    uint8_t Unknown71[0x7b - 0x71];
    uint8_t Flags7B;
    uint8_t Unknown7C[0x82 - 0x7c];
    uint8_t Flags82;
    uint8_t Unknown83[0x8b - 0x83];
    uint8_t Flags8B;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of "cmp left, right" on a dword.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "or byte, imm" (CF = OF = AF = 0).
static inline uint32_t orFlags8(uint32_t value) {
    return parityFlag8(value)
        | ((value & 0xffu) == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00432a0f
// pushal ... popal wrapper on the car record in esi: clears bit 4 of the bytes +0x7b and +0x8b,
// runs the resets 0x00432933 and 0x004326ca, and unless the sign byte 0x007c2ea9 is negative
// runs 0x00408c07 (and 0x0043cc1d when that byte is zero) and sets byte +0x70 to 1. If esi is the
// car pointer in 0x00679d2c, bit 0 of byte +0x82 is set. popal restores every register; the flags
// are those of the final cmp, or of the `or` when the car matched.
void __cdecl FUN_00432a0f(gp4::Regs* r) {
    Unknown_00432a0f_Car* car = reinterpret_cast<Unknown_00432a0f_Car*>(static_cast<uintptr_t>(r->esi));

    car->Flags7B &= 0xefu;
    car->Flags8B &= 0xefu;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00432933, call);
    gp4::call_regs(0x004326ca, call);

    if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) == 0u) {
        gp4::call_regs(0x00408c07, call);
        if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0u) {
            gp4::call_regs(0x0043cc1d, call);
        }
        car->Flag70 = 1;
    }

    uint32_t flags = cmpFlags32(r->esi, GP4_GLOBAL(uint32_t, 0x00679d2c));
    if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
        car->Flags82 = static_cast<uint8_t>(car->Flags82 | 1u);
        flags = orFlags8(car->Flags82);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432a0f, FUN_00432a0f, "regs(ecx:u32[0..3], esi:ptr[0x358]:layout=rec_00410894+rec_004087aa) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00410894+state_004087aa+state_0066d8c0+counter_0040f0dd+trk_007ad894+state_0040f112+state_00432a8b+div_0060138c+circ_007adb50+codes_0043ba12+curves_0062ecb0+state_0040fbf3")
