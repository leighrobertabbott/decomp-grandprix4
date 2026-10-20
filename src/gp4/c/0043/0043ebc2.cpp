#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ebc2
void __cdecl FUN_0043ebc2(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->edi);

    gp4::Regs call = *r;
    gp4::call_regs(0x0043eb86, call);
    GP4_FIELD(uint16_t, body, 0xc8) = static_cast<uint16_t>(call.eax);

    const int32_t divisor = static_cast<int16_t>(GP4_FIELD(uint16_t, body, 0x14));
    uint32_t flags;
    if (divisor == 0) {
        GP4_FIELD(uint16_t, body, 0x52) = 0;
        GP4_FIELD(uint16_t, body, 0x54) = 0;
        // `or ebp, ebp` with ebp == 0.
        flags = 0x44u;
    } else {
        const int32_t numerator = 0x400 << 14;
        const int32_t quotient = numerator / divisor;

        call = *r;
        call.ebp = static_cast<uint32_t>(quotient);
        call.edx = static_cast<uint32_t>(numerator % divisor);
        call.eax = static_cast<uint32_t>(GP4_FIELD(uint16_t, body, 0x60)) + 0x50u + static_cast<uint32_t>(quotient);
        gp4::call_regs(0x0043eb86, call);
        GP4_FIELD(uint16_t, body, 0x52) = static_cast<uint16_t>(call.eax);

        call.eax = static_cast<uint32_t>(quotient) - (static_cast<uint32_t>(GP4_FIELD(uint16_t, body, 0x62)) + 0x50u);
        gp4::call_regs(0x0043eb86, call);
        GP4_FIELD(uint16_t, body, 0x54) = static_cast<uint16_t>(call.eax);
        flags = call.eflags;
    }

    // popal restores every register; only the flags of the last operation survive.
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0043ebc2, FUN_0043ebc2, "regs(eax:i32, edi:ptr[0xd8]:layout=body_0043ebc2) -> (cf, pf, af, zf, sf, of)")
