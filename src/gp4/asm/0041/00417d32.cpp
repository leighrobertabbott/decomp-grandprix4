#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by an OR/TEST: CF = OF = AF = 0, PF from the low byte, ZF/SF from the result of the
// given width.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// One of the four 0x300-byte slots of the record array at 0x006380d8: a dword stamp at +0x1d4 and a
// status byte at +0x2e.
struct Unknown_00417d32_Slot {
    uint8_t Unknown00[0x2e];
    uint8_t Flags2E;
    uint8_t Unknown2F[0x1d4 - 0x2f];
    uint32_t Unknown1D4;
    uint8_t Unknown1D8[0x300 - 0x1d8];
};

struct Unknown_00417d32_Source {
    uint8_t Unknown00[0x40];
    uint32_t Unknown40;
};

// FUNCTION: GP4 0x00417d32
// For each of four bits in the dword at esi+0x40 (0x10000, 0x4000, 0x200, 0x80, in that order) that
// is set: stores the stamp DAT_00601e14 into the matching slot of the record array at 0x006380d8,
// sets bit 5 of that slot's status byte and sets DAT_00601f1d to 0xff. Leaves ebx = 0x006380d8,
// eax = the stamp, ecx = the dword; the flags are those of the last OR/TEST executed.
void __cdecl FUN_00417d32(gp4::Regs* r) {
    static const uint32_t masks[4] = {0x10000u, 0x4000u, 0x200u, 0x80u};
    const Unknown_00417d32_Source* source =
        reinterpret_cast<const Unknown_00417d32_Source*>(static_cast<uintptr_t>(r->esi));
    Unknown_00417d32_Slot* slots = reinterpret_cast<Unknown_00417d32_Slot*>(GP4_ARRAY(uint8_t, 0x006380d8));

    const uint32_t stamp = GP4_GLOBAL(uint32_t, 0x00601e14);
    const uint32_t bits = source->Unknown40;
    uint32_t flags = logicFlags32(bits);

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slots));
    r->eax = stamp;
    r->ecx = bits;

    if (bits != 0u) {
        for (int i = 0; i < 4; ++i) {
            flags = logicFlags32(bits & masks[i]);
            if ((bits & masks[i]) != 0u) {
                slots[i].Unknown1D4 = stamp;
                slots[i].Flags2E = static_cast<uint8_t>(slots[i].Flags2E | 0x20u);
                flags = logicFlags8(slots[i].Flags2E);
                GP4_GLOBAL(uint8_t, 0x00601f1d) = 0xffu;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00417d32, FUN_00417d32, "regs(esi:ptr[0x48]:bytes) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
