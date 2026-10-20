#include <gp4/gp4.h>

static inline uint32_t subtractionFlags16(uint16_t left, uint16_t right) {
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

// FUNCTION: GP4 0x0043b51c
// Stores (angle - 0x14) to word 0x66d88a, where angle is the 11-bit word at (body->+0x1c)+0x1a, or,
// when bit 2 of body byte +0x8d is set, the result of 0x405985 (with cx = sign-extended byte +0x67)
// wrapped once by the word 0x868558. Leaves edi = [ebx+0x1c]; flags are those of the final sub.
void __cdecl FUN_0043b51c(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* link = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, body, 0x1c)));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(link));

    uint32_t eax = (r->eax & 0xffff0000u) | (GP4_FIELD(uint16_t, link, 0x1a) & 0x7ffu);

    if ((GP4_FIELD(uint8_t, body, 0x8d) & 2u) != 0u) {
        // mov cl,[ebx+0x67]; xchg cx,ax; cbw; xchg cx,ax -> cx = sign-extended byte, ax unchanged
        const int16_t offset = static_cast<int16_t>(GP4_FIELD(int8_t, body, 0x67));
        gp4::Regs callee = *r;
        callee.st_in = 0;
        callee.st_out = 0;
        callee.eax = eax;
        callee.ecx = (r->ecx & 0xffff0000u) | static_cast<uint16_t>(offset);
        gp4::call_regs(0x00405985, callee);
        eax = callee.eax;
        r->ecx = callee.ecx;
        r->edx = callee.edx;

        const int16_t wrap = GP4_GLOBAL(int16_t, 0x00868558);
        if (static_cast<int16_t>(eax) >= wrap) {
            eax = (eax & 0xffff0000u) | static_cast<uint16_t>(static_cast<uint16_t>(eax) - static_cast<uint16_t>(wrap));
        }
    }

    const uint16_t angle = static_cast<uint16_t>(eax);
    const uint16_t result = static_cast<uint16_t>(angle - 0x14u);
    GP4_GLOBAL(uint16_t, 0x0066d88a) = result;

    r->eax = (eax & 0xffff0000u) | result;
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags16(angle, 0x14u);
}
GP4_IMPL(0x0043b51c, FUN_0043b51c, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
