#include <gp4/gp4.h>

static inline uint32_t Parity(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ~parity & 1u;
}

// flags of a 32-bit subtraction lhs - rhs = result
static inline uint32_t SubFlags32(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (lhs < rhs ? 1u : 0u) | (Parity(result) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040b264
void __cdecl FUN_0040b264(gp4::Regs* r) {
    uint8_t* const start = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* out = start;

    // rep movsb x3: three global blocks are appended to the caller's buffer
    for (uint32_t i = 0u; i != 0x007adb4cu - 0x007ad890u; ++i) {
        *out++ = GP4_ARRAY(uint8_t, 0x007ad890)[i];
    }
    for (uint32_t i = 0u; i != 0x007c3ae4u - 0x007c28bcu; ++i) {
        *out++ = GP4_ARRAY(uint8_t, 0x007c28bc)[i];
    }
    for (uint32_t i = 0u; i != 0x006975bcu - 0x0066d7a4u; ++i) {
        *out++ = GP4_ARRAY(uint8_t, 0x0066d7a4)[i];
    }

    // mov eax, edi ; sub eax, edx(saved edi): total bytes written
    const uint32_t end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    const uint32_t begin = r->edi;
    const uint32_t written = end - begin;
    r->eax = written;
    r->eflags = (r->eflags & ~0x8d5u) | SubFlags32(end, begin, written);
}
GP4_IMPL(0x0040b264, FUN_0040b264, "regs(edi:ptr[176900]:bytes) -> (eax, cf, pf, af, zf, sf, of); wall=1200")
