#include <gp4/gp4.h>

// Car record (esi): dword +0x98 a speed, word +0x9a a signed 16-bit value (clamped to <= 0x5800 here).
// Updates the speed from the two lookups 0x00426a66 / 0x00425f41, then re-derives word +0x9a as the integer
// square root (0x00409c44) of  (word delta * [0x604116] ... / [0x603854]) << 10  +  word^2, where the
// 16-bit quotient is only taken when it provably fits (the 0x43db53.. range checks against half the
// divisor); otherwise it counts as 0. eax and edx leave as 0x00409c44 left them (their upper halves come
// from the speed update), every other register and the flags of 0x00409c44 are what the caller sees.
// FUNCTION: GP4 0x0043da95
void __cdecl FUN_0043da95(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    // movsx ebx, word [esi+0x9a]; clamp the word (signed) to 0x5800 and store it back
    const int32_t original = (int32_t)(int16_t)GP4_FIELD(uint16_t, rec, 0x9a);
    uint16_t clamped = (uint16_t)original;
    if ((int16_t)clamped > 0x5800) {
        clamped = 0x5800;
    }
    GP4_FIELD(uint16_t, rec, 0x9a) = clamped;

    gp4::call_regs(0x00426a66, c);
    GP4_GLOBAL(uint16_t, 0x00934df4) = 0xc000;
    gp4::call_regs(0x00425f41, c);

    // eax = (lookup << 2) - [0x6037fc]; speed += eax, not below zero
    const uint32_t delta = (c.eax << 2) - GP4_GLOBAL(uint32_t, 0x006037fc);
    int32_t speed = (int32_t)(GP4_FIELD(uint32_t, rec, 0x98) + delta);
    if (speed < 0) {
        speed = 0;
    }
    GP4_FIELD(int32_t, rec, 0x98) = speed;

    // ax = word[+0x9a] - bx; dx:ax = ax * (int16)[0x604116]; ax = bits 14..29 of that product
    const uint16_t diff = (uint16_t)(GP4_FIELD(uint16_t, rec, 0x9a) - clamped);
    const int32_t product = (int32_t)(int16_t)diff * (int32_t)(int16_t)GP4_GLOBAL(uint16_t, 0x00604116);
    const uint16_t scaled = (uint16_t)(((uint32_t)product << 2) >> 16);

    // square of the (unclamped) word: imul ax -> cx:bx
    const int32_t word = (int32_t)(int16_t)original;
    const uint32_t square = (uint32_t)(word * word);

    // dx:ax = (-scaled * 0x800) << 6
    const int32_t numerator = (int32_t)(int16_t)(uint16_t)(0u - scaled) * 0x800;
    const uint32_t dividend = (uint32_t)numerator << 6;
    const int16_t high = (int16_t)(dividend >> 16);
    const uint16_t low = (uint16_t)dividend;

    // bp = half the divisor word (negative values get +1), compared against the high word of the dividend
    const int16_t divisor = (int16_t)GP4_GLOBAL(uint32_t, 0x00603854);
    int16_t half = (int16_t)(divisor >> 1);
    if (half < 0) {
        half = (int16_t)(half + 1);
    }
    bool divide;
    if (high < 0) {
        const int16_t bound = half >= 0 ? (int16_t)(0 - half) : half;
        divide = high > bound || (high == bound && low != 0u);
    } else if (half < 0) {
        divide = high < (int16_t)(0 - half);
    } else {
        divide = high < half;
    }

    int32_t quotient = 0;
    if (divide) {
        quotient = (int32_t)dividend / (int32_t)divisor;
    }

    // dx:ax = quotient << 10, plus the square; its integer square root goes to word +0x9a
    const uint32_t radicand = (uint32_t)((int32_t)(int16_t)quotient * 1024) + square;
    c.eax = (delta & 0xffff0000u) | (radicand & 0xffffu);
    c.edx = ((uint32_t)speed & 0xffff0000u) | (radicand >> 16);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00409c44, c);
    GP4_FIELD(uint16_t, rec, 0x9a) = (uint16_t)c.eax;

    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043da95, FUN_0043da95, "regs(esi:ptr[0x252]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00425f41")
