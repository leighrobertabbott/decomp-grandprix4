#include <gp4/gp4.h>

// Flags of an 8-bit logic result (CF = OF = AF = 0): parity of the low byte, ZF, SF.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t p = result & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return ((p & 1u) == 0u ? 0x04u : 0u)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// FUNCTION: GP4 0x004036ca
// pushal wrapper over a car record at esi: with the track fraction [0x601b34] temporarily set to
// the sign-extended word +0x66, classify the car's segment position (0x00403a1e, stored to byte
// +0x294), turn the word +0x94 into an offset (0x00403938) and compare the word +0x90 against it
// (0x00403ab9, stored to dword +0x1a0); the sign half of that result sets (-1) or clears bit 3 of
// byte +0x71. popal restores every register, so the flags of that final and/or are the output.
void __cdecl FUN_004036ca(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)r->esi;
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x66);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.edi = GP4_FIELD(uint32_t, rec, 0x1c);
    c.eax = c.edi;
    gp4::call_regs(0x00403a1e, c);
    GP4_FIELD(uint8_t, rec, 0x294) = (uint8_t)c.eax;

    c.edx = c.eax;
    c.eax = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x94);
    gp4::call_regs(0x00403938, c);

    c.ecx = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x90);
    gp4::call_regs(0x00403ab9, c);
    GP4_FIELD(uint32_t, rec, 0x1a0) = c.eax;

    uint8_t flagsByte = GP4_FIELD(uint8_t, rec, 0x71);
    if (c.edx == 0xffffffffu) {
        flagsByte = (uint8_t)(flagsByte | 0x08u);
    } else {
        flagsByte = (uint8_t)(flagsByte & 0xf7u);
    }
    GP4_FIELD(uint8_t, rec, 0x71) = flagsByte;

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(flagsByte);
}
GP4_IMPL(0x004036ca, FUN_004036ca, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+links_00405a12+state_00403800+div_00403bb6+ringw_004291f4+carf_004036ca")
