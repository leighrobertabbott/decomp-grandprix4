#include <gp4/gp4.h>

struct Unknown_00416c72 {
    uint8_t Unknown00[0x18];
    uint32_t Unknown18;
    int32_t Unknown1C;
    uint32_t Unknown20;
    uint8_t Unknown24[0x0c];
    int32_t Unknown30;
};

// FUNCTION: GP4 0x00416c72
// Stores min((|[ebx+0x18]| << 18) / max(DAT_006034a4, (|[ebx+0x18]| >> 14) + 1), 0x400000) at [ebx+0x20]
// and the angle (0x00401240) of the vector (x = [ebx+0x18], y = +-divisor, negated when [ebx+0x1c] is
// negative) as a sign-extended word at [ebx+0x30]. eax = that angle, edx = y, ebp = divisor; the flags
// are those the angle routine leaves.
void __cdecl FUN_00416c72(gp4::Regs* r) {
    Unknown_00416c72* body = reinterpret_cast<Unknown_00416c72*>(static_cast<uintptr_t>(r->ebx));

    const uint32_t x = body->Unknown18;
    uint32_t magnitude = x;
    if (static_cast<int32_t>(magnitude) < 0) magnitude = 0u - magnitude;

    uint32_t high = magnitude >> 14;   // shld edx, eax, 18 with edx = 0
    const uint32_t low = magnitude << 18;
    uint32_t divisor = GP4_GLOBAL(uint32_t, 0x006034a4);
    if (!(divisor > high)) {
        divisor = high + 1u;
        if (divisor == 0u) {
            divisor -= 1u;
            high -= 1u;
        }
    }

    uint32_t remainder;
    uint32_t quotient = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);
    if (quotient > 0x400000u) quotient = 0x400000u;
    body->Unknown20 = quotient;

    uint32_t y = divisor;
    if (body->Unknown1C < 0) y = 0u - y;

    gp4::Regs call = *r;
    call.eax = x;
    call.edx = y;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401240, call);

    const int32_t angle = static_cast<int32_t>(static_cast<int16_t>(call.eax));
    body->Unknown30 = angle;

    r->eax = static_cast<uint32_t>(angle);
    r->edx = y;
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00416c72, FUN_00416c72, "regs(ebx:ptr[0x40]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
