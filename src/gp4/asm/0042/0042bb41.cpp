#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a 16-bit "cmp a, b" (a - b).
static inline uint32_t cmpFlags16(uint16_t a, uint16_t b) {
    const uint16_t res = (uint16_t)(a - b);
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x1u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (res == 0u) f |= 0x40u;
    if ((res & 0x8000u) != 0u) f |= 0x80u;
    if ((((a ^ b) & (a ^ res)) & 0x8000u) != 0u) f |= 0x800u;
    return f;
}

// Flags of an 8-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint8_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if ((res & 0x80u) != 0u) f |= 0x80u;
    return f;
}

// FUNCTION: GP4 0x0042bb41
void __cdecl FUN_0042bb41(gp4::Regs* r) {
    const int16_t ax = (int16_t)r->eax;
    const uint8_t* edi = (const uint8_t*)r->edi;
    const uint8_t* ebx = (const uint8_t*)r->ebx;
    uint8_t* esi = (uint8_t*)r->esi;

    int16_t limit = (int16_t)GP4_GLOBAL(uint16_t, 0x00603b44);
    uint32_t flags = cmpFlags16((uint16_t)ax, (uint16_t)limit);
    if (ax < limit) {
        if ((edi[0x51] & 0x80u) != 0u) {
            limit = (int16_t)GP4_GLOBAL(uint16_t, 0x00603da8);
        } else {
            limit = (int16_t)GP4_GLOBAL(uint16_t, 0x00603b40);
        }
        flags = cmpFlags16((uint16_t)ax, (uint16_t)limit);
        if (!(ax < limit)) {
            const uint8_t t = (uint8_t)(ebx[0xcc] & 0x10u);
            flags = logicFlags8(t);
            if (t == 0u) {
                esi[0xcb] = (uint8_t)(esi[0xcb] | 2u);
                flags = logicFlags8(esi[0xcb]);
                const int16_t limit2 = (int16_t)GP4_GLOBAL(uint16_t, 0x00603b42);
                flags = cmpFlags16((uint16_t)ax, (uint16_t)limit2);
                if (!(ax < limit2)) {
                    const int16_t w = *(const int16_t*)(esi + 0x9a);
                    flags = cmpFlags16((uint16_t)w, 0x0b00u);
                    if (!(w < 0x0b00)) {
                        esi[0xca] = (uint8_t)(esi[0xca] | 2u);
                        flags = logicFlags8(esi[0xca]);
                    }
                }
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042bb41, FUN_0042bb41, "regs(eax:u32, ebx:ptr[0xd4], esi:ptr[0xd4], edi:ptr[0x5c]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
