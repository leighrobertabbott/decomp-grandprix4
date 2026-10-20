#include <gp4/gp4.h>

// A 16-bit position below half the lap length wraps up by the lap length (word 0x00868558), then
// is shifted to 18.14 fixed point.
static inline uint32_t unwrapPosition(uint32_t position) {
    if (static_cast<int32_t>(position) < static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x00868558) >> 1)) {
        position += GP4_GLOBAL(uint16_t, 0x00868558);
    }
    return position << 14;
}

// FUNCTION: GP4 0x0040d8a5
// pushal ... popal routine. Walks the 0x160-byte ring records from DAT_00604428 to DAT_0060442c
// (wrapping at DAT_00866ecc back to 0x008943a0). For each, finds in the table of 8-byte entries at
// 0x00604435 (indexed by word +0x7a of the cursor record ebx, advanced by 0x160 until the next
// entry's start lies above the record's position) the interpolation fraction of the record's
// position, splits it with 0x00405985 and stores the two result words into the entry
// belonging to the current record. Only the flags of the final "cmp edi, [0x0060442c]" (equal)
// escape.
void __cdecl FUN_0040d8a5(gp4::Regs* r) {
    uint8_t* edi = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00604428));
    uint8_t* ebx = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00604424));
    int32_t rem;

    for (;;) {
        const int32_t position = static_cast<int32_t>(unwrapPosition(GP4_FIELD(uint16_t, edi, 0x1a) & 0x7ffu));

        uint8_t* entry;
        for (;;) {
            entry = GP4_ARRAY(uint8_t, 0x00604435) + static_cast<uint32_t>(GP4_FIELD(uint16_t, ebx, 0x7a)) * 8u;
            GP4_GLOBAL(int32_t, 0x00604420) = GP4_FIELD(int16_t, entry, 2) + static_cast<int32_t>(unwrapPosition(GP4_FIELD(uint16_t, entry, 0)));
            GP4_GLOBAL(int32_t, 0x0060441c) = GP4_FIELD(int16_t, entry, 0xa) + static_cast<int32_t>(unwrapPosition(GP4_FIELD(uint16_t, entry, 8)));
            if (position < GP4_GLOBAL(int32_t, 0x0060441c)) {
                break;
            }
            ebx += 0x160;
        }

        const int32_t offset = position - GP4_GLOBAL(int32_t, 0x00604420);
        int32_t span = GP4_GLOBAL(int32_t, 0x0060441c) - GP4_GLOBAL(int32_t, 0x00604420);
        if (span == 0) {
            span = 1;
        }
        const int32_t fraction = gp4::idiv64(static_cast<int64_t>(offset) << 14, span, &rem);

        uint8_t* target = GP4_ARRAY(uint8_t, 0x00604435) + static_cast<uint32_t>(GP4_FIELD(uint16_t, edi, 0x7a)) * 8u;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = (static_cast<uint32_t>(fraction) & 0xffff0000u) | (GP4_FIELD(uint16_t, ebx, 0x1a) & 0x7ffu);
        c.ecx = (reinterpret_cast<uint32_t>(entry) & 0xffff0000u)
            | static_cast<uint16_t>(static_cast<int16_t>(fraction) >> 8);
        gp4::call_regs(0x00405985, c);

        int16_t whole = static_cast<int16_t>(c.eax);
        const uint16_t sub = static_cast<uint16_t>(static_cast<uint16_t>(c.ecx) << 8);
        if (whole >= static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00868558))) {
            whole = static_cast<int16_t>(static_cast<uint16_t>(whole) - GP4_GLOBAL(uint16_t, 0x00868558));
        }
        GP4_FIELD(int16_t, target, 4) = whole;
        GP4_FIELD(uint16_t, target, 6) = sub;

        edi += 0x160;
        if (reinterpret_cast<uint32_t>(edi) == GP4_GLOBAL(uint32_t, 0x0060442c)) {
            break;
        }
        if (reinterpret_cast<uint32_t>(edi) >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            edi = GP4_ARRAY(uint8_t, 0x008943a0);
        }
    }

    // popal restores the registers; the last flag writer is "cmp edi, [0x0060442c]" with equality
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040d8a5, FUN_0040d8a5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040d8a5")
