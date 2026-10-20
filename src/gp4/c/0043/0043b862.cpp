#include <gp4/gp4.h>

// Lap fraction of the car whose number is the byte at 0x007c31dc, in 1/256 lap units rounded to
// nearest: ((lapWord * wrapLength - offset) clamped at 0) << 8, plus half the wrap length, divided by
// the wrap length (0x00868558). The offset is the 32-bit dx:ax of 0x0043b482 for the matching car
// record (0x0043b83d searches the 22 records; none found counts as 0). All registers but eax are
// preserved; the flags are those of the `xor edx,edx` before the div, which leaves them untouched.
// FUNCTION: GP4 0x0043b862
void __cdecl FUN_0043b862(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.eax = (r->eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007c31dc);
    gp4::call_regs(0x0043b83d, call);

    uint32_t offset = 0u;
    if ((call.eflags & 1u) == 0u) {
        gp4::call_regs(0x0043b482, call);
        // shl eax, 16; mov ax, dx; rol eax, 16
        offset = ((call.edx & 0xffffu) << 16) | (call.eax & 0xffffu);
    }

    const uint32_t wrap = GP4_GLOBAL(uint16_t, 0x00868558);
    uint32_t value = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) * wrap - offset;
    if (static_cast<int32_t>(value) < 0) {
        value = 0u;
    }
    value = (value << 8) + (wrap >> 1);
    r->eax = value / wrap;
    // xor edx, edx -> ZF and PF set, everything else clear; div keeps them
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043b862, FUN_0043b862, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lap_0043b482+carno_0066ff6c+state_0040d8a5")
