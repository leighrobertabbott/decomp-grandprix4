#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of "test word, 0xffff": CF = OF = AF = 0.
static inline uint32_t testFlags16(uint16_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0041ae14
void __cdecl FUN_0041ae14(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;

    GP4_FIELD(uint16_t, body, 0xa0) = 0x4000u;

    uint32_t eax = r->eax;
    const uint8_t first = body[0x72];
    eax = (eax & 0xffffff00u) | first;
    const uint8_t second = body[0x121];
    if (first != second) {
        eax = (eax & 0xffffff00u) | second;
        if (second > first) {
            // call 0x401462 (eax = shifter state input); bit 0 of al decides the flag
            gp4::Regs shifter = *r;
            shifter.eax = eax;
            gp4::call_regs(0x00401462, shifter);
            if (((shifter.eax & 0xffu) & 1u) == 0u) {
                body[0x86] = (uint8_t)(body[0x86] | 0x80u);
            }
        }
        eax = GP4_GLOBAL(uint32_t, 0x00602fbc);
        GP4_FIELD(uint16_t, body, 0x1ae) = (uint16_t)eax;
    }

    const uint16_t counter = GP4_FIELD(uint16_t, body, 0x1ae);
    const uint32_t flags = testFlags16(counter);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    if (counter != 0u) {
        gp4::Regs consume = *r;
        consume.eax = eax;
        consume.edx = GP4_GLOBAL(uint32_t, 0x0060384c);
        gp4::call_regs(0x0041ae6c, consume);
        r->eax = consume.eax;
        r->edx = consume.edx;
        r->ebp = consume.ebp;
        r->eflags = consume.eflags;
    } else {
        r->eax = eax;
    }
}
GP4_IMPL(0x0041ae14, FUN_0041ae14, "regs(eax:u32, esi:ptr[0x1b6]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
