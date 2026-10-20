#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Opcode 9 of the opcode-stream reader (dispatched through 0x00601270 by 0x004452f1): the read cursor
// 0x0062d1a8 (ebx) points at the opcode byte followed by four bytes. Byte +1 is OR-ed into 0x008914e0, byte
// +2 is the flag byte applied by 0x00445248, bytes {0x008914e2, +3} are fed to 0x004454ef as al / ah, and the
// cursor advances by 5. eax ends as {upper bits, +3, +4}; ebx keeps the cursor; the flags are those of the
// final `add dword [0x62d1a8], 5`.
// FUNCTION: GP4 0x00444eb3
void __cdecl FUN_00444eb3(gp4::Regs* r) {
    const uint8_t* stream = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a8)));
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint32_t upper = r->eax & 0xffff0000u;

    GP4_GLOBAL(uint8_t, 0x008914e0) = static_cast<uint8_t>(stream[1] | GP4_GLOBAL(uint8_t, 0x008914e0));

    gp4::Regs flagCall = *r;
    flagCall.eax = (r->eax & 0xffffff00u) | stream[2];
    flagCall.ebx = cursor;
    gp4::call_regs(0x00445248, flagCall);

    gp4::Regs sampleCall = *r;
    sampleCall.ebx = cursor;
    sampleCall.eax = (flagCall.eax & 0xffff0000u) | (static_cast<uint32_t>(stream[3]) << 8) | GP4_GLOBAL(uint8_t, 0x008914e2);
    gp4::call_regs(0x004454ef, sampleCall);

    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 5u;

    r->eax = upper | (static_cast<uint32_t>(stream[3]) << 8) | stream[4];
    r->ebx = cursor;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursor, 5u);
}
GP4_IMPL(0x00444eb3, FUN_00444eb3, "regs(eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a8")
