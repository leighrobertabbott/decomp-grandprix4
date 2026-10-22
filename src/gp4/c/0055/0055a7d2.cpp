#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mirror of 0x0055a71c: track-segment ring pass over the records of 0x160 bytes (lap start
// [0x891504], lap end [0x866ecc], pit range [0x891500, 0x866ef8), wrap distance [0x866ec4]). For
// ([0x8705e8] + 0x100 + 1) records, starting at the lap start and stepping BACKWARD (clamped at the
// pit start, wrapping by the distance below the lap start), a record whose byte +0x17 is 1..0x3b
// starts a FORWARD walk of up to 0x3c - value records (the forward step wraps at the lap end and
// stops at the pit end). On the way, the counter (value, value + 1, ...) is stored into byte +0x20
// of every record whose flag +0x1a bit 0 is SET and whose +0x20 is 0x3c or more or greater than
// the counter. The routine leaves al = the last +0x17 byte read, cx = 0x3c once any walk ran (else
// 0), dx = 0xffff (the loop ends on the sign of `sub dx, 1`), edi = the record the pass ended on
// and the flags of the last `sub dx, 1`.
// FUNCTION: GP4 0x0055a7d2
void __cdecl FUN_0055a7d2(gp4::Regs* r) {
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00891504);
    uint16_t dx = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705e8) + 0x100u);
    uint16_t cx = 0;
    uint8_t al;
    uint32_t flags;

    do {
        al = GP4_FIELD(uint8_t, edi, 0x17);
        if (al != 0u && al < 0x3cu) {
            uint32_t walk = edi;
            cx = al;
            do {
                if ((GP4_FIELD(uint8_t, walk, 0x1a) & 1u) != 0u) {
                    const uint8_t old = GP4_FIELD(uint8_t, walk, 0x20);
                    if (old >= 0x3cu || static_cast<uint8_t>(cx) < old) {
                        GP4_FIELD(uint8_t, walk, 0x20) = static_cast<uint8_t>(cx);
                    }
                }
                cx = static_cast<uint16_t>(cx + 1u);
                walk += 0x160u;
                if (walk >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    if (walk < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        walk -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                    } else if (walk >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                        walk = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                    }
                }
            } while (static_cast<int16_t>(cx) < 0x3c);
        }
        if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi -= 0x160u;
            if (edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
                edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            edi -= 0x160u;
            if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                edi = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
        flags = gp4::flags::sub16(dx, 1u);
        dx = static_cast<uint16_t>(dx - 1u);
    } while (static_cast<int16_t>(dx) >= 0);

    r->eax = (r->eax & 0xffffff00u) | al;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055a7d2, FUN_0055a7d2, "regs(eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+walk_0055a71c")
