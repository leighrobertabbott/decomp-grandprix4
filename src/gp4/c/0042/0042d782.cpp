#include <gp4/gp4.h>

// Flags of a 16-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0042d782
void __cdecl FUN_0042d782(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->ebx);
    const uint32_t edxIn = r->edx;
    uint16_t dx;
    uint16_t rhs;
    uint32_t flags;
    bool fail;

    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00604153)) >= 0) {
        dx = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0x90) + 0x200);
        rhs = GP4_FIELD(uint16_t, body, 0x11e);
        flags = cmpFlags16(dx, rhs);
        fail = static_cast<int16_t>(dx) > static_cast<int16_t>(rhs);
        if (!fail) {
            dx = GP4_GLOBAL(uint16_t, 0x00604404);
            rhs = GP4_GLOBAL(uint16_t, 0x00603e62);
            flags = cmpFlags16(dx, rhs);
            fail = static_cast<int16_t>(dx) < static_cast<int16_t>(rhs);
        }
    } else {
        dx = static_cast<uint16_t>(static_cast<uint16_t>(-GP4_FIELD(uint16_t, body, 0x90)) + 0x200);
        rhs = GP4_FIELD(uint16_t, body, 0x11c);
        flags = cmpFlags16(dx, rhs);
        fail = static_cast<int16_t>(dx) > static_cast<int16_t>(rhs);
        if (!fail) {
            dx = GP4_GLOBAL(uint16_t, 0x00604404);
            rhs = GP4_GLOBAL(uint16_t, 0x00603e60);
            flags = cmpFlags16(dx, rhs);
            fail = static_cast<int16_t>(dx) > static_cast<int16_t>(rhs);
        }
    }

    r->edx = (edxIn & 0xffff0000u) | dx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (fail ? 1u : 0u);
}
GP4_IMPL(0x0042d782, FUN_0042d782, "regs(ebx:ptr[0x128]:bytes, edx:u32) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
