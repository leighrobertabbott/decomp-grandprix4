#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of a byte-wide logic result (test / and / or / cmp reg,0): CF = OF = AF = 0
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// flags of a dword-wide logic result (or eax,eax)
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Toggles the flag bits of one car/driver record. Returns the flags of the last
// logic instruction executed (the final or/and on a flag byte).
static inline uint32_t toggleRecord(uint8_t* rec) {
    if ((rec[0x71] & 0x80u) == 0u) {
        rec[0x71] = static_cast<uint8_t>(rec[0x71] | 0x80u);
        rec[0xfe] = static_cast<uint8_t>(rec[0xfe] | 0x04u);
        rec[0x230] = static_cast<uint8_t>(rec[0x230] & 0x7fu);
        return logicFlags8(rec[0x230]);
    }
    rec[0x71] = static_cast<uint8_t>(rec[0x71] & 0x7fu);
    rec[0xfe] = static_cast<uint8_t>(rec[0xfe] & 0xfbu);
    rec[0x231] = static_cast<uint8_t>(rec[0x231] | 0x20u);
    return logicFlags8(rec[0x231]);
}

// FUNCTION: GP4 0x0041104f
// Toggles the flag bits (+0x71 bit 7, +0xfe bit 2, +0x230 / +0x231) of the record
// at pointer global 0x0066d820 and, when byte 0x007c2e6c is set, of the record at
// 0x0066d82c, skipping null pointers and records with +0x8d bit 1 set.
// eax and esi are saved and restored; only the flags escape.
void __cdecl FUN_0041104f(gp4::Regs* r) {
    uint8_t* const first = GP4_GLOBAL(uint8_t*, 0x0066d820);
    if (first != nullptr && (first[0x8d] & 0x02u) == 0u) {
        toggleRecord(first);
    }

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    uint32_t flags = logicFlags8(enabled);   // cmp byte [0x7c2e6c], 0
    if (enabled != 0u) {
        uint8_t* const second = GP4_GLOBAL(uint8_t*, 0x0066d82c);
        flags = logicFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second)));
        if (second != nullptr) {
            flags = logicFlags8(second[0x8d] & 0x02u);
            if ((second[0x8d] & 0x02u) == 0u) {
                flags = toggleRecord(second);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041104f, FUN_0041104f, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0041104f")
