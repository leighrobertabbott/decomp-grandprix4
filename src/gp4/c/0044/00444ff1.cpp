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

// Opcode 0x0d of the opcode-stream reader (reads what 0x00444f84 writes): the read cursor 0x0062d1a8 (ebx)
// points at a ten-byte record. When the car [0x0066d828] (esi) is not null, bytes +1..+5 go to car +0x198,
// +0x136, +0x199, +0x19a, +0x19b and bytes +6/+7 to the word +0x60; 0x00408783 runs, byte +8 goes to +0x7f,
// 0x00410b09 runs, byte +9 goes to the table 0x0066d936 entry for slot ((+0x7c & 0x3f) - 1, capped at 0x15)
// and 0x004415c9 runs. The cursor advances by 10 in every case; ebx keeps the old cursor, esi the car.
// FUNCTION: GP4 0x00444ff1
void __cdecl FUN_00444ff1(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));
    const uint32_t carAddr = GP4_GLOBAL(uint32_t, 0x0066d828);
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(carAddr));

    r->ebx = cursor;
    r->esi = carAddr;
    if (car != nullptr) {
        const uint32_t upper = r->eax & 0xffff0000u;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;

        car->Byte198 = record[1];
        car->Byte136 = record[2];
        car->Byte199 = record[3];
        car->Byte19A = record[4];
        car->Byte19B = record[5];
        car->Word60 = static_cast<uint16_t>(record[6] | (static_cast<uint32_t>(record[7]) << 8));
        c.eax = upper | (static_cast<uint32_t>(record[7]) << 8) | record[6];
        gp4::call_regs(0x00408783, c);

        car->Byte7F = record[8];
        c.eax = upper | (static_cast<uint32_t>(record[7]) << 8) | record[8];
        gp4::call_regs(0x00410b09, c);

        int32_t slot = static_cast<int32_t>(car->Slot7C & 0x3fu) - 1;
        if (slot > 0x15) {
            slot = 0x15;
        }
        c.edx = static_cast<uint32_t>(slot);
        c.eax = upper | (static_cast<uint32_t>(record[7]) << 8) | record[9];
        GP4_ARRAY(uint8_t, 0x0066d936)[slot] = record[9];
        gp4::call_regs(0x004415c9, c);

        r->eax = c.eax;
        r->edx = c.edx;
    }

    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 0x0au;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursor, 0x0au);
}
GP4_IMPL(0x00444ff1, FUN_00444ff1, "regs(eax:u32, edx:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+carno_0066ff6c+stream_00444ff1+state_0043c1ba+state_004320f2")
