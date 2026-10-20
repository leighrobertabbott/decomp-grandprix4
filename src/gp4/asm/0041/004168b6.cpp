#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Address of the dword at byte offset `offset` from `base` (kept out of line so the compiler does
// not fold base + offset into separate absolute globals).
static __declspec(noinline) uint32_t* slotAt(uint8_t* base, uint32_t offset) {
    return reinterpret_cast<uint32_t*>(base + offset);
}

// Copies the dword at esi+0x3c into the two dwords at 0x006380d8 + 0x600 and + 0x900 (each is
// "(0 << 16) + value"). Leaves ebx = 0x006380d8, eax = edx = the copied value. The flags are
// those of the final "add eax, edx" with eax = 0: CF = AF = OF = 0, the rest from the value.
// FUNCTION: GP4 0x004168b6
void __cdecl FUN_004168b6(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x3c);

    *slotAt(base, 0x600) = value;
    *slotAt(base, 0x900) = value;

    r->ebx = reinterpret_cast<uint32_t>(base);
    r->edx = value;
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}
GP4_IMPL(0x004168b6, FUN_004168b6, "regs(esi:ptr[0x44]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
