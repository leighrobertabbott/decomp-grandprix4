#include <gp4/gp4.h>
#include <gp4/flags.h>

// One readout digit cell: `mov eax, 0; mov al, byte; [shr al, 4;] and al, 0xf; add al, 0x30`.
static inline uint32_t DigitChar(uint32_t byteValue, bool high) {
    const uint32_t nibble = high ? ((byteValue >> 4) & 0xfu) : (byteValue & 0xfu);
    return nibble + 0x30u;
}

// FUNCTION: GP4 0x0051547e
// Formats the lap-time dwords of a car record esi for the readout (event handler, no stack
// arguments): clears the four cursor bytes 0x88fd94..97, sets 0x88fe14 = 4, takes the time
// t = dword [esi + 0x20 + 4 * byte [esi + 0x74]] and, below 600000 ms, converts it to packed BCD
// through 0x0051fd6f (kept in 0x88fde0) and stores the six digit characters '0'+nibble in
// 0x8660dc..0x8660f0 (bytes 2, 1, 0 of the BCD, high nibble first); otherwise six '-' (0x2d).
// Then 0x88fe14 = -1, bit 4 of the byte +0x88 of the record is cleared and 0x954169 = 0xff.
// pushal/popal around the body: only eax = t and ecx = the index byte differ from the entry
// registers; the flags are those of the final `and byte [esi + 0x88], 0xef`.
void __cdecl FUN_0051547e(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0u;
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 4u;

    const uint32_t record = r->esi;
    const uint32_t index = GP4_FIELD(uint8_t, record, 0x74);
    const int32_t time = GP4_FIELD(int32_t, record + index * 4u, 0x20);

    if (time < 600000) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = static_cast<uint32_t>(time);
        c.ecx = index;
        gp4::call_regs(0x0051fd6f, c);
        GP4_GLOBAL(uint32_t, 0x0088fde0) = c.eax;

        const uint32_t hours = GP4_GLOBAL(uint8_t, 0x0088fde2);
        GP4_GLOBAL(uint32_t, 0x008660dc) = DigitChar(hours, true);
        GP4_GLOBAL(uint32_t, 0x008660e0) = DigitChar(hours, false);
        const uint32_t minutes = GP4_GLOBAL(uint8_t, 0x0088fde1);
        GP4_GLOBAL(uint32_t, 0x008660e4) = DigitChar(minutes, true);
        GP4_GLOBAL(uint32_t, 0x008660e8) = DigitChar(minutes, false);
        const uint32_t seconds = GP4_GLOBAL(uint8_t, 0x0088fde0);
        GP4_GLOBAL(uint32_t, 0x008660ec) = DigitChar(seconds, true);
        GP4_GLOBAL(uint32_t, 0x008660f0) = DigitChar(seconds, false);
    } else {
        GP4_GLOBAL(uint32_t, 0x008660dc) = 0x2du;
        GP4_GLOBAL(uint32_t, 0x008660e0) = 0x2du;
        GP4_GLOBAL(uint32_t, 0x008660e4) = 0x2du;
        GP4_GLOBAL(uint32_t, 0x008660e8) = 0x2du;
        GP4_GLOBAL(uint32_t, 0x008660ec) = 0x2du;
        GP4_GLOBAL(uint32_t, 0x008660f0) = 0x2du;
    }

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    const uint8_t flagByte = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x88) & 0xefu);
    GP4_FIELD(uint8_t, record, 0x88) = flagByte;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;

    r->eax = static_cast<uint32_t>(time);
    r->ecx = index;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagByte));
}
GP4_IMPL(0x0051547e, FUN_0051547e, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=times_00513f45")
