#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the cursor 0x00679bdc / 0x006f3a38. Bit 7 of the byte at 0x007ad88e selects the
// value of 0x006f3a34 as the candidate; otherwise bit 6 of 0x0066d7a4 selects [0x679bdc] + 1
// (wrapping to 1 at the limit 0x00679c24) and a clear bit selects [0x6f3a38] unchanged. The
// candidate (eax) is stored in both globals; the flags are those of the last compare/test.
// FUNCTION: GP4 0x005249ca
void __cdecl FUN_005249ca(gp4::Regs* r) {
    int32_t value;
    uint32_t flags;
    bool compare = true;
    if ((GP4_GLOBAL(uint8_t, 0x007ad88e) & 0x80u) != 0u) {
        value = GP4_GLOBAL(int32_t, 0x006f3a34);
    } else {
        const uint8_t bits = (uint8_t)(GP4_GLOBAL(uint8_t, 0x0066d7a4) & 0x40u);
        if (bits == 0u) {
            value = GP4_GLOBAL(int32_t, 0x006f3a38);
            compare = false;
            flags = gp4::flags::logic8(bits);
        } else {
            value = GP4_GLOBAL(int32_t, 0x00679bdc) + 1;
        }
    }
    if (compare) {
        const int32_t limit = GP4_GLOBAL(int32_t, 0x00679c24);
        flags = gp4::flags::sub32((uint32_t)value, (uint32_t)limit);
        if (!(value < limit)) {
            value = 1;
        }
    }
    GP4_GLOBAL(int32_t, 0x00679bdc) = value;
    GP4_GLOBAL(int32_t, 0x006f3a38) = value;
    r->eax = (uint32_t)value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005249ca, FUN_005249ca, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
