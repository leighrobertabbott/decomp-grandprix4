#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043d7be
// Re-integrates the speed word +0x9a of a car record (esi): the word is clamped to 0x5800, the
// dword +0x98 (whose high half overlaps +0x9a) is advanced by the two update routines
// (0x426a66, 0x425eca) and floored at zero, and the new +0x9a is the integer square root
// (0x409c44) of (old speed)^2 plus a 16:16 quotient correction built from the change of the
// overlapping word and [0x603854]. ebx, ecx and ebp are saved and restored; eax and edx are left
// by the square-root routine. Its closing "add esp, 0x20" makes the flags a function of the
// stack pointer: PF only in the pinned oracle model (as 0x409c44).
void __cdecl FUN_0043d7be(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int32_t original = GP4_FIELD(int16_t, rec, 0x9a);   // movsx eax, word [esi+0x9a]
    int16_t limited = static_cast<int16_t>(original);
    if (limited > 0x5800) {
        limited = 0x5800;
    }
    GP4_FIELD(int16_t, rec, 0x9a) = limited;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.eax = static_cast<uint32_t>(original);
    s.ebx = (r->ebx & 0xffff0000u) | static_cast<uint16_t>(limited);
    gp4::call_regs(0x00426a66, s);

    GP4_GLOBAL(uint16_t, 0x00934df4) = 0x6000;
    const uint32_t angle = GP4_FIELD(uint16_t, rec, 0xee);
    GP4_GLOBAL(uint32_t, 0x00603d40) = angle;
    GP4_GLOBAL(uint32_t, 0x00603d48) = 0;
    s.eax = angle;
    gp4::call_regs(0x00425eca, s);

    const uint32_t adjust = (s.eax << 2) - GP4_GLOBAL(uint32_t, 0x006037fc);
    uint32_t advanced = GP4_FIELD(uint32_t, rec, 0x98) + adjust;
    if (static_cast<int32_t>(advanced) < 0) {
        advanced = 0;
    }
    GP4_FIELD(uint32_t, rec, 0x98) = advanced;

    // The dword store above rewrote the word at +0x9a; its change against the clamped speed
    // is scaled by 0x800 and shifted left by 6 into a 16:16 numerator (low word always zero).
    const uint16_t change = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x9a) - static_cast<uint16_t>(limited));
    const uint32_t numerator = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(change)) * 0x800) << 6;
    const int16_t numeratorHigh = static_cast<int16_t>(numerator >> 16);
    const uint16_t numeratorLow = static_cast<uint16_t>(numerator);

    // Half of the low word of [0x603854], rounded towards zero for negative values.
    const uint32_t scale = GP4_GLOBAL(uint32_t, 0x00603854);
    int16_t half = static_cast<int16_t>(static_cast<int16_t>(scale) >> 1);
    if (half < 0) {
        half = static_cast<int16_t>(half + 1);
    }
    const int32_t magnitude = half < 0 ? -half : half;

    // The 32/16 division only runs when the quotient is certain to fit 16 bits.
    bool divide;
    if (numeratorHigh < 0) {
        divide = numeratorHigh > -magnitude || (numeratorHigh == -magnitude && numeratorLow != 0u);
    } else {
        divide = numeratorHigh < magnitude;
    }
    int32_t quotient = 0;
    if (divide) {
        quotient = static_cast<int16_t>(static_cast<int32_t>(numerator) / static_cast<int32_t>(static_cast<int16_t>(scale)));
    }

    // dx:ax = quotient sign-extended and shifted left by 10, plus the square of the old speed.
    const int32_t speed = static_cast<int16_t>(original);
    const uint32_t square = static_cast<uint32_t>(speed * speed);
    const uint32_t value = (static_cast<uint32_t>(quotient) << 10) + square;

    s.eax = (adjust & 0xffff0000u) | (value & 0xffffu);
    s.edx = (advanced & 0xffff0000u) | (value >> 16);
    s.ebx = (static_cast<uint32_t>(original) & 0xffff0000u) | (square & 0xffffu);
    s.ecx = (r->ecx & 0xffff0000u) | (square >> 16);
    s.ebp = scale;
    gp4::call_regs(0x00409c44, s);

    GP4_FIELD(uint16_t, rec, 0x9a) = static_cast<uint16_t>(s.eax);
    r->eax = s.eax;
    r->edx = s.edx;
    r->eflags = (r->eflags & ~0x8d5u) | 0x04u;
}
GP4_IMPL(0x0043d7be, FUN_0043d7be, "regs(esi:ptr[0x267]:layout=car_0043d7be) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043d7be+torque_00425e19+frac_00624790")
