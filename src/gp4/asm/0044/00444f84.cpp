#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x60];
    uint16_t Word60;
    uint8_t Unknown62[0x1a];
    uint8_t Slot7C;
    uint8_t Unknown7D[2];
    uint8_t Byte7F;
    uint8_t Unknown80[0xb6];
    uint8_t Byte136;
    uint8_t Unknown137[0x61];
    uint8_t Byte198;
    uint8_t Byte199;
    uint8_t Byte19A;
    uint8_t Byte19B;
};

// Appends a ten-byte record (opcode 0x0d, then bytes +0x198, +0x136, +0x199, +0x19a, +0x19b, the word +0x60
// low byte first, +0x7f and the table 0x0066d936 entry for slot ((+0x7c & 0x3f) - 1, capped at 0x15) of the
// car in esi) to the opcode stream whose write cursor is the pointer 0x0062d1a0 and advances the cursor by
// 10. push / pop restore eax and ebx; only the flags of the final "add dword [cursor], 10" survive.
// FUNCTION: GP4 0x00444f84
void __cdecl FUN_00444f84(gp4::Regs* r) {
    const Unknown_Car* car = reinterpret_cast<const Unknown_Car*>(static_cast<uintptr_t>(r->esi));
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));

    cursor[0] = 0x0du;
    cursor[1] = car->Byte198;
    cursor[2] = car->Byte136;
    cursor[3] = car->Byte199;
    cursor[4] = car->Byte19A;
    cursor[5] = car->Byte19B;
    const uint16_t word = car->Word60;
    cursor[6] = static_cast<uint8_t>(word);
    cursor[7] = static_cast<uint8_t>(word >> 8);
    cursor[8] = car->Byte7F;
    int32_t slot = static_cast<int32_t>(car->Slot7C & 0x3fu) - 1;
    if (slot > 0x15) {
        slot = 0x15;
    }
    cursor[9] = GP4_ARRAY(const uint8_t, 0x0066d936)[slot];

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 0x0au;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 0x0au);
}
GP4_IMPL(0x00444f84, FUN_00444f84, "regs(esi:ptr[0x1a3]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0+counter_0040f0dd")
