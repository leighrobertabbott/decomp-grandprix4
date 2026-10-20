#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine, esi = car record (0x0066fef0 array, stride 0x41c). Updates a small
// four-character readout (dwords 0x008660f8..0x00866104, cached value 0x0088fdb4):
// - mode byte 0x007c2ea9 == 0x80 and the car pointer [0x0066d820] is set, differs from esi and has
//   bit 7 of +0x7a clear: the readout becomes "----" (cache 0x40000000) and "--.-" is sent through
//   0x0051f3bd (al = character, edi = output cursor);
// - otherwise the value (word +0x60, or 0x004080c9 when bit 4 of +0x8b is set) is shown: 0x46, then
//   the BCD digits of min(value >> 6, 99) via 0x0051fcdb (leading blank 0x20), then the digit
//   ((value & 0x3f) * 10) >> 6.
// Both paths return early when the cache already holds the value (flags of that equal compare);
// otherwise 0x0088fe14 = -1, bit 7 of car byte +0x89 cleared, 0x00954169 = 0xff.
static void send_char(gp4::Regs& c, uint8_t ch) {
    c.eax = (c.eax & 0xffffff00u) | ch;
    gp4::call_regs(0x0051f3bd, c);
}

static uint32_t car_value(const gp4::Regs* r, uint8_t* car, uint32_t eax, bool movzx) {
    if ((GP4_FIELD(uint8_t, car, 0x8b) & 0x10u) != 0) {
        gp4::Regs c = {};
        c.eax = eax;
        c.ebx = r->ebx;
        c.ecx = r->ecx;
        c.edx = r->edx;
        c.esi = r->esi;
        c.edi = r->edi;
        c.ebp = r->ebp;
        c.eflags = r->eflags;
        gp4::call_regs(0x004080c9, c);
        return c.eax;
    }
    return (movzx ? 0u : (eax & 0xffff0000u)) | GP4_FIELD(uint16_t, car, 0x60);
}

// FUNCTION: GP4 0x00515187
void __cdecl FUN_00515187(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const other = GP4_GLOBAL(uint8_t*, 0x0066d820);

    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u && car != other && other != 0 &&
        (GP4_FIELD(uint8_t, other, 0x7a) & 0x80u) == 0) {
        if (GP4_GLOBAL(uint32_t, 0x0088fdb4) == 0x40000000u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x40000000u, 0x40000000u));
            return;
        }
        GP4_GLOBAL(uint32_t, 0x0088fdb4) = 0x40000000u;
        GP4_GLOBAL(uint32_t, 0x0088fe14) = 1;
        GP4_GLOBAL(uint8_t, 0x0088fd94) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd95) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;
        GP4_GLOBAL(uint32_t, 0x008660f8) = 0x2d;
        GP4_GLOBAL(uint32_t, 0x008660fc) = 0x2d;
        GP4_GLOBAL(uint32_t, 0x00866100) = 0x2d;
        GP4_GLOBAL(uint32_t, 0x00866104) = 0x2d;

        gp4::Regs c = {};
        c.eax = r->eax;
        c.ebx = r->ebx;
        c.ecx = r->ecx;
        c.edx = r->edx;
        c.esi = r->esi;
        c.edi = r->edi;
        c.ebp = r->ebp;
        c.eflags = r->eflags;
        send_char(c, 0x2d);
        send_char(c, 0x2d);
        send_char(c, 0x2e);
        send_char(c, 0x2d);
        r->eax = c.eax;
        r->ebx = c.ebx;
        r->ecx = c.ecx;
        r->edx = c.edx;
        r->esi = c.esi;
        r->edi = c.edi;
        r->ebp = c.ebp;
    } else {
        GP4_GLOBAL(uint32_t, 0x008660f8) = 0x46;
        const uint32_t value = car_value(r, car, r->eax, true);
        if (value == GP4_GLOBAL(uint32_t, 0x0088fdb4)) {
            r->eax = value;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(value, value));
            return;
        }
        GP4_GLOBAL(uint32_t, 0x0088fdb4) = value;
        uint32_t shown = value >> 6;
        if (static_cast<int32_t>(shown) > 0x63) {
            shown = 0x63;
        }
        GP4_GLOBAL(uint32_t, 0x0088fe14) = 1;
        GP4_GLOBAL(uint8_t, 0x0088fd94) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd95) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
        GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;

        gp4::Regs b = {};
        b.eax = shown;
        b.ebx = r->ebx;
        b.ecx = r->ecx;
        b.edx = r->edx;
        b.esi = r->esi;
        b.edi = r->edi;
        b.ebp = r->ebp;
        b.eflags = r->eflags;
        gp4::call_regs(0x0051fcdb, b);
        const uint8_t bcd = static_cast<uint8_t>(b.eax);
        const uint8_t tens = static_cast<uint8_t>(bcd >> 4);
        if (tens == 0) {
            GP4_GLOBAL(uint32_t, 0x008660fc) = 0x20;
        } else {
            GP4_GLOBAL(uint32_t, 0x008660fc) = static_cast<uint8_t>(tens + 0x30u);
        }
        GP4_GLOBAL(uint32_t, 0x00866100) = static_cast<uint8_t>((bcd & 0x0fu) + 0x30u);

        GP4_GLOBAL(uint8_t, 0x0088fd96) = 5;
        const uint32_t frac = car_value(r, car, shown, false);
        const uint32_t digit = (((frac & 0x3fu) * 5u) << 1) >> 6;
        const uint32_t out = (digit & 0xffffff00u) | static_cast<uint8_t>(digit + 0x30u);
        GP4_GLOBAL(uint32_t, 0x00866104) = out & 0xffu;
        r->eax = out;
    }

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    const uint8_t b89 = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x89) & 0x7fu);
    GP4_FIELD(uint8_t, car, 0x89) = b89;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b89));
}
GP4_IMPL(0x00515187, FUN_00515187, "regs(esi:rec[0x66fef0,0x41c,0..21], edi:ptr[0x10], eax:u32) -> (eax, edi, cf, pf, af, zf, sf, of); globals=layout=readout_00515187")
