#include <gp4/gp4.h>

// FUNCTION: GP4 0x00415491
void __cdecl FUN_00415491(gp4::Regs* r) {
    const int32_t value = static_cast<int32_t>(r->eax);
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    int32_t scale;
    if ((GP4_FIELD(uint8_t, body, 0x51) & 0x20) == 0 || (GP4_FIELD(uint8_t, body, 0x51) & 0x80) != 0) {
        scale = 0x4000;
    } else {
        const int64_t wide = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062fbb4));
        const uint32_t shifted = (static_cast<uint32_t>(wide) >> 14) | (static_cast<uint32_t>(static_cast<uint64_t>(wide) >> 32) << 18);
        scale = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062fbb0) - shifted);
        if (scale < 0) {
            scale = 0;
        }
        scale += 0x4000;
    }
    GP4_GLOBAL(int32_t, 0x0062fbc4) = scale;

    // final imul edx / shrd eax, edx, 14 produce the result and the flags
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(scale) * static_cast<int64_t>(value));
    const uint32_t result = (static_cast<uint32_t>(product) >> 14) | (static_cast<uint32_t>(product >> 32) << 18);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = static_cast<uint32_t>((product >> 13) & 1u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415491, FUN_00415491, "regs(eax:i32, edi:ptr[0x59]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
