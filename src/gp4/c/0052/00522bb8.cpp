#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the car-code list: [0x0066d8ec] bytes from 0x0066d8f4 to 0x00675958 (`test ecx, ecx ; je`
// skips an empty list, then a `loop`). esi, ebx and ecx are saved and restored; al holds the last byte
// copied. The flags are those of the `test`, or of the last `inc esi` (CF kept from the `test`).
// FUNCTION: GP4 0x00522bb8
void __cdecl FUN_00522bb8(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t arith = gp4::flags::logic32(count);
    if (count != 0u) {
        const uint8_t* src = GP4_ARRAY(uint8_t, 0x0066d8f4);
        uint8_t* dst = GP4_ARRAY(uint8_t, 0x00675958);
        uint8_t last = 0u;
        uint32_t i = 0u;
        do {
            last = src[i];
            dst[i] = last;
            ++i;
        } while (i != count);
        arith = gp4::flags::inc32(0x0066d8f4u + count - 1u, arith);
        r->eax = (r->eax & 0xffffff00u) | last;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522bb8, FUN_00522bb8, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00443278")
