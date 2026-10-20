#include <gp4/gp4.h>

struct Unknown_0066fef0 {   // one of the 22 car records (stride 0x41c)
    uint8_t Unknown00[0x7a];
    uint8_t Flags7A;        // bit 7: record is out
    uint8_t Unknown7B[0xfd - 0x7b];
    uint8_t UnknownFD;      // key compared with the low byte of eax
    uint8_t UnknownFE[0x41c - 0xfe];
};
static_assert(sizeof(Unknown_0066fef0) == 0x41c, "record stride");

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043b8be
// Finds the first and the second of the 22 car records at 0x0066fef0 whose byte +0xfd equals al.
// eax = the first record's address, edx = the second's; -1 when there is none or its +0x7a has bit 7
// set. ebx, ecx and esi are preserved. The flags are those of the last "add esi, 0x41c" when a
// scan runs off the end, or of the "test byte [esi+0x7a], 0x80" of a second hit.
void __cdecl FUN_0043b8be(gp4::Regs* r) {
    Unknown_0066fef0* records = reinterpret_cast<Unknown_0066fef0*>(static_cast<uintptr_t>(0x0066fef0u));
    const uint8_t wanted = static_cast<uint8_t>(r->eax);

    const uint32_t lastRecord = 0x0066fef0u + 21u * 0x41cu;
    uint32_t flags = additionFlags32(lastRecord, 0x41cu);   // a scan ran off the end
    uint32_t first = 0xffffffffu;
    uint32_t second = 0xffffffffu;

    uint32_t index = 0;
    while (index != 22u && records[index].UnknownFD != wanted) {
        ++index;
    }

    if (index != 22u) {
        if ((records[index].Flags7A & 0x80u) == 0u) {
            first = reinterpret_cast<uint32_t>(&records[index]);
        }
        ++index;
        while (index != 22u && records[index].UnknownFD != wanted) {
            ++index;
        }
        if (index != 22u) {
            const uint32_t test = records[index].Flags7A & 0x80u;
            if (test == 0u) {
                second = reinterpret_cast<uint32_t>(&records[index]);
            }
            // TEST: CF = OF = AF = 0
            flags = parityFlag8(test) | (test == 0u ? 0x40u : 0u) | (test & 0x80u);
        }
    }

    r->eax = first;
    r->edx = second;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043b8be, FUN_0043b8be, "regs(eax:u32[0..30]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
