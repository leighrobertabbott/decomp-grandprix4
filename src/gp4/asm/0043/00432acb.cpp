#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of an add / sub / test of the given width. `mask` is 0xff or 0xffffffff, `sign` the top bit.
static inline uint32_t resultFlags(uint32_t res, uint32_t sign) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & sign) != 0u ? 0x80u : 0u);
}
static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t mask, uint32_t sign) {
    const uint32_t res = (a + b) & mask;
    uint32_t f = resultFlags(res, sign);
    if (res < a) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if ((~(a ^ b) & (a ^ res) & sign) != 0u) f |= 0x800u;
    return f;
}
static inline uint32_t subFlags(uint32_t a, uint32_t b, uint32_t mask, uint32_t sign) {
    const uint32_t res = (a - b) & mask;
    uint32_t f = resultFlags(res, sign);
    if (a < b) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (((a ^ b) & (a ^ res) & sign) != 0u) f |= 0x800u;
    return f;
}

// Scheduling timer of a car record (esi): byte +0x25d is the step (0xff = start, 0..3), dword +0x258 the
// millisecond time ([0x66d814] clock) at which the next step is due, byte +0x25c a mode, dword +0xf0 a limit.
// CF is set when the routine returns "not finished" (stc), clear after step 3 (clc).
// FUNCTION: GP4 0x00432acb
void __cdecl FUN_00432acb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    uint32_t flags;
    bool carry = true;

    if (GP4_FIELD(uint8_t, record, 0x25d) == 0xffu) {
        gp4::call_regs(0x00401462, c);
        const uint32_t add = (c.eax & 0xffu) << 1;
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, record, 0x258) = clock + add;
        flags = addFlags(clock, add, 0xffffffffu, 0x80000000u);
        c.eax = add;
        GP4_FIELD(uint8_t, record, 0x25d) = 0;
    } else if (GP4_FIELD(uint32_t, record, 0x258) == 0u) {
        // test dword, -1
        flags = 0x44u;
    } else {
        const uint32_t due = GP4_FIELD(uint32_t, record, 0x258);
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t negDue = 0u - due;
        flags = addFlags(negDue, clock, 0xffffffffu, 0x80000000u);
        if ((flags & 0x80u) == 0u) {
            gp4::call_regs(0x00401462, c);
            uint32_t eax = (c.eax & 0xffu) + 0x12cu;
            const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_FIELD(uint32_t, record, 0x258) = now + eax;
            flags = addFlags(now, eax, 0xffffffffu, 0x80000000u);
            flags = subFlags(GP4_FIELD(uint8_t, record, 0x25c), 2u, 0xffu, 0x80u);
            if (GP4_FIELD(uint8_t, record, 0x25c) == 2u) {
                flags = subFlags(GP4_FIELD(uint8_t, record, 0x25d), 2u, 0xffu, 0x80u);
                if (GP4_FIELD(uint8_t, record, 0x25d) == 2u) {
                    const uint32_t limit = GP4_FIELD(uint32_t, record, 0xf0);
                    eax = limit - 0x2bcu;
                    const uint32_t cur = GP4_FIELD(uint32_t, record, 0x258);
                    flags = subFlags(cur, eax, 0xffffffffu, 0x80000000u);
                    if (static_cast<int32_t>(cur) > static_cast<int32_t>(eax)) {
                        GP4_FIELD(uint32_t, record, 0x258) = eax;
                    }
                }
            }
            c.eax = eax;
            c.eflags = (c.eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x00418c39, c);
            const uint32_t step = GP4_FIELD(uint8_t, record, 0x25d);
            flags = subFlags(step, 3u, 0xffu, 0x80u);
            if (step == 3u) {
                carry = false;
            } else {
                flags = addFlags(step, 1u, 0xffu, 0x80u);
                GP4_FIELD(uint8_t, record, 0x25d) = static_cast<uint8_t>(step + 1u);
            }
        }
    }
    // stc leaves CF set, clc clears it; everything else keeps the flags of the last writer
    flags = carry ? (flags | 0x01u) : (flags & ~0x01u);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00432acb, FUN_00432acb, "regs(eax:u32, esi:ptr[0x268]:layout=pit_00432acb) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_00418c68+clock_0066d814")
