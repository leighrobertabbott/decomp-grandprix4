#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte-wide logic result (test / or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_0042f450_Link {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
};

struct Unknown_0042f450 {
    uint8_t Unknown00[0x1c];
    Unknown_0042f450_Link* Link1C;
    uint8_t Unknown20[0x21c - 0x20];
    uint8_t Flags21C;
};

// Mirrors bit 7 of the linked record's byte +0x51 into bit 0 of +0x21c: set it when the bit is
// set; otherwise, if bit 0 was set, clear it and add DAT_006223e8 to the clamped word at +0xb6
// (0x0042a0cf). edi is saved and restored.
// FUNCTION: GP4 0x0042f450
void __cdecl FUN_0042f450(gp4::Regs* r) {
    Unknown_0042f450* self = reinterpret_cast<Unknown_0042f450*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042f450_Link* link = self->Link1C;

    uint32_t flags = logicFlags8(link->Flags51 & 0x80u);
    if ((link->Flags51 & 0x80u) != 0u) {
        self->Flags21C = static_cast<uint8_t>(self->Flags21C | 0x01u);
        flags = logicFlags8(self->Flags21C);
    } else {
        flags = logicFlags8(self->Flags21C & 0x01u);
        if ((self->Flags21C & 0x01u) != 0u) {
            self->Flags21C = static_cast<uint8_t>(self->Flags21C & 0xfeu);
            gp4::Regs callee = *r;
            callee.st_in = 0;
            callee.st_out = 0;
            callee.eax = GP4_GLOBAL(uint32_t, 0x006223e8);
            gp4::call_regs(0x0042a0cf, callee);
            r->eax = callee.eax;
            flags = callee.eflags & 0x8d5u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f450, FUN_0042f450, "regs(esi:ptr[0x224]:layout=rec_0042f450) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0042f450")
