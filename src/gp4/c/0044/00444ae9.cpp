#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Appends one 10-byte record for the car [0x0066d820] to the opcode stream whose write cursor is the
// pointer at 0x0062d1a0 (opcode 0, state bits from 0x445185, car byte +0x6f masked with the mode byte
// 0x008914e0, word +0x6a, bytes +0x6e/+0x6c/+0x6d, flag byte from 0x44520a, byte 0x008914e2) and advances
// the cursor by 10. With no car the five-byte record of 0x444e81 is appended instead. pushal / popal
// restore every register; the flags are those of the final add in the callee or here.
// FUNCTION: GP4 0x00444ae9
void __cdecl FUN_00444ae9(gp4::Regs* r) {
    const uint32_t carAddress = GP4_GLOBAL(uint32_t, 0x0066d820);

    if (carAddress == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00444e81, call);
        r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
        return;
    }

    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(carAddress));
    const uint32_t cursorAddress = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    uint8_t* stream = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursorAddress));

    stream[0] = 0u;

    gp4::Regs bits = *r;
    bits.esi = carAddress;
    bits.ebx = cursorAddress;
    bits.st_in = 0;
    bits.st_out = 0;
    gp4::call_regs(0x00445185, bits);
    stream[1] = static_cast<uint8_t>(bits.eax);

    stream[2] = static_cast<uint8_t>((car[0x6f] & 0x9fu) | (GP4_GLOBAL(uint8_t, 0x008914e0) & 0x60u));
    *reinterpret_cast<uint16_t*>(stream + 3) = *reinterpret_cast<const uint16_t*>(car + 0x6a);
    stream[5] = car[0x6e];
    stream[6] = car[0x6c];
    stream[7] = car[0x6d];

    gp4::Regs flag = *r;
    flag.esi = carAddress;
    flag.ebx = cursorAddress;
    flag.eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x008914e0) & 0x60u) << 8) | car[0x6d];
    flag.st_in = 0;
    flag.st_out = 0;
    gp4::call_regs(0x0044520a, flag);
    stream[8] = static_cast<uint8_t>(flag.eax);
    stream[9] = GP4_GLOBAL(uint8_t, 0x008914e2);

    GP4_GLOBAL(uint32_t, 0x0062d1a0) = cursorAddress + 0x0au;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursorAddress, 0x0au);
}
GP4_IMPL(0x00444ae9, FUN_00444ae9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+carx_0042fe2f+state_00413f28+state_0040fa01+stream_0062d1a0")
