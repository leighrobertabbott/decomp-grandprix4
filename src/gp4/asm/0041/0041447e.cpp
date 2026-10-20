#include <gp4/gp4.h>

static inline int32_t word(const uint8_t* base, uint32_t off) {
    return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(base + off));
}

// Angle (FUN_00401315) from the offset between the 2D point at esi and a point taken from the record six
// entries (0x840 bytes, wrapped at the lap/pit limits) after the record at edi. The point is built from the words at
// +4/+8 (times 8) plus the words at +0xd6/+0xde or +0xd4/+0xdc, scaled by 256. ebx, edx and edi are preserved,
// eax and the flags are those of FUN_00401315.
// FUNCTION: GP4 0x0041447e
void __cdecl FUN_0041447e(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint8_t* origin = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t next = static_cast<uint32_t>(r->edi);
    if (next >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        next += 0x840u;
        if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            next = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    } else {
        next += 0x840u;
        if (next >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            next -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }
    const uint8_t* target = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(next));

    const uint32_t flags = rec[0x51];
    bool useAlternate;  // words +0xd6/+0xde (true) or +0xd4/+0xdc (false)
    if ((flags & 0x20u) != 0u) {
        if ((flags & 0x80u) != 0u) {
            useAlternate = (flags & 8u) != 0u;
        } else {
            useAlternate = (flags & 8u) == 0u;
        }
    } else {
        useAlternate = *reinterpret_cast<const int16_t*>(target + 0x16) < 0;
    }

    const uint32_t xOff = useAlternate ? 0xd6u : 0xd4u;
    const uint32_t yOff = useAlternate ? 0xdeu : 0xdcu;
    const uint32_t dx = (static_cast<uint32_t>(word(target, 4) * 8 + word(target, xOff)) << 8) -
                        *reinterpret_cast<const uint32_t*>(origin);
    const uint32_t dy = (static_cast<uint32_t>(word(target, 8) * 8 + word(target, yOff)) << 8) -
                        *reinterpret_cast<const uint32_t*>(origin + 4);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = dx;
    c.edx = dy;
    gp4::call_regs(0x00401315, c);

    r->eax = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0041447e, FUN_0041447e, "regs(edi:rec[0x8943a0,0x160,0..29], esi:ptr[8]:i32[-0x1000000..0x1000000]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
