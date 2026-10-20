#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes one nibble channel of the record ebx and advances the running word 0x00874b54. When the
// byte 0x00874b3e is zero it only runs 0x0055926c (the plain nibble store). Otherwise the signed
// word 0x00874b54 * 32 is divided by the record's word +0x60 (negative value) or +0x62 (a zero
// divisor becomes 1) and the low byte goes to +0x1d when dl == 0x40 (high nibble 0x6f3dfb[[0x874b4e]]
// << 4 into +0x25) or to +0x1c (low nibble); dl is also or-ed into +0x92. Finally 0x00874b54 += the
// word 0x00874b56. pushal/popal keeps every register except the low word of eax (= 0x00874b56); the
// flags are those of the final `add word [0x874b54], ax` (or of 0x0055926c).
// FUNCTION: GP4 0x00559287
void __cdecl FUN_00559287(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x00874b3e) == 0u) {
        gp4::call_regs(0x0055926c, *r);
        return;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint8_t dl = static_cast<uint8_t>(r->edx);

    const int16_t position = GP4_GLOBAL(int16_t, 0x00874b54);
    uint32_t divisor = (position < 0) ? GP4_FIELD(uint16_t, record, 0x60) : GP4_FIELD(uint16_t, record, 0x62);
    if (divisor == 0u) {
        divisor = 1u;
    }
    const int32_t quotient = (static_cast<int32_t>(position) << 5) / static_cast<int32_t>(divisor);

    uint8_t nibble = GP4_ARRAY(uint8_t, 0x006f3dfb)[GP4_GLOBAL(uint8_t, 0x00874b4e)];
    if (dl == 0x40u) {
        record[0x1d] = static_cast<uint8_t>(quotient);
        nibble = static_cast<uint8_t>(nibble << 4);
        record[0x25] = static_cast<uint8_t>(record[0x25] & 0x0fu);
    } else {
        record[0x1c] = static_cast<uint8_t>(quotient);
        nibble = static_cast<uint8_t>(nibble & 0x0fu);
        record[0x25] = static_cast<uint8_t>(record[0x25] & 0xf0u);
    }
    record[0x25] = static_cast<uint8_t>(record[0x25] | nibble);
    record[0x92] = static_cast<uint8_t>(record[0x92] | dl);

    const uint16_t step = GP4_GLOBAL(uint16_t, 0x00874b56);
    const uint16_t before = GP4_GLOBAL(uint16_t, 0x00874b54);
    GP4_GLOBAL(uint16_t, 0x00874b54) = static_cast<uint16_t>(before + step);

    r->eax = (r->eax & 0xffff0000u) | step;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add16(before, step));
}
GP4_IMPL(0x00559287, FUN_00559287, "regs(eax:u32, edx:u32{0x40,0x20,0,0xff,0x141}, ebx:ptr[0xa0]:u16{0,0,1,2,0x40,0x100,0x7fff,0xffff}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
