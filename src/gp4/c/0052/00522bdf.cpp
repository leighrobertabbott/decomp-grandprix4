#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the order list: [0x0066d8ec] bytes from 0x00675958 to 0x007c31f2 (`or ecx, ecx ; je` skips
// an empty list, then a `loop`). ecx ends 0, al holds the last byte copied, ebx is saved and
// restored. The flags are those of `or ecx, ecx`, or of the last `inc ebx` (CF kept from the `or`).
// FUNCTION: GP4 0x00522bdf
void __cdecl FUN_00522bdf(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t arith = gp4::flags::logic32(count);
    if (count != 0u) {
        const uint8_t* src = GP4_ARRAY(uint8_t, 0x00675958);
        uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c31f2);
        uint8_t last = 0u;
        uint32_t i = 0u;
        do {
            last = src[i];
            dst[i] = last;
            ++i;
        } while (i != count);
        arith = gp4::flags::inc32(count - 1u, arith);
        r->eax = (r->eax & 0xffffff00u) | last;
    }
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522bdf, FUN_00522bdf, "regs(eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+list_00443278")
