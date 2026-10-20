#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Loads a saved car record at esi into the car array: rebases the record's pointer fields (0x40b186), then
// requires both segment pointers (+0x1c, +0x1b8) to lie in [0x008943a0, [0x00866ef8]). On success it toggles
// bit 6 of byte +0x7c when bit 7 is set, copies [0x0062f8b8] bytes (whole dwords) to the car slot
// 0x0066fef0 + byte(+0x202) * 0x41c and finishes that copy with 0x418af8. CF is clear on success (esi ends
// at the end of the source copy). On failure CF is set, eax holds the offending pointer and the other flags
// are those of the failed compare.
// FUNCTION: GP4 0x0044580c
void __cdecl FUN_0044580c(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs rebase = *r;
    rebase.st_in = 0;
    rebase.st_out = 0;
    gp4::call_regs(0x0040b186, rebase);

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);

    uint32_t pointer = GP4_FIELD(uint32_t, record, 0x1c);
    if (pointer < 0x008943a0u) {
        r->eax = pointer;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(pointer, 0x008943a0u) & ~1u) | 1u;
        return;
    }
    if (pointer >= limit) {
        r->eax = pointer;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(pointer, limit) & ~1u) | 1u;
        return;
    }

    pointer = GP4_FIELD(uint32_t, record, 0x1b8);
    if (pointer < 0x008943a0u) {
        r->eax = pointer;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(pointer, 0x008943a0u) & ~1u) | 1u;
        return;
    }
    if (pointer >= limit) {
        r->eax = pointer;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(pointer, limit) & ~1u) | 1u;
        return;
    }

    if ((GP4_FIELD(uint8_t, record, 0x7c) & 0x80u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x7c) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x7c) ^ 0x40u);
    }

    const uint32_t slot = 0x0066fef0u + static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 0x202)) * 0x41cu;
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062f8b8) >> 2;
    const uint32_t* source = reinterpret_cast<const uint32_t*>(record);
    uint32_t* dest = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(slot));
    for (uint32_t i = 0; i < count; ++i) {
        dest[i] = source[i];
    }

    gp4::Regs finish = *r;
    finish.eax = slot;
    finish.ecx = 0u;
    finish.edx = 0u;
    finish.esi = slot;
    finish.edi = slot + count * 4u;
    finish.st_in = 0;
    finish.st_out = 0;
    gp4::call_regs(0x00418af8, finish);

    r->eax = finish.eax;
    r->ecx = finish.ecx;
    r->edx = finish.edx;
    r->ebx = finish.ebx;
    r->ebp = finish.ebp;
    r->edi = finish.edi;
    r->esi = r->esi + count * 4u;
    r->eflags = (r->eflags & ~0x8d5u) | (finish.eflags & 0x8d4u);
}
GP4_IMPL(0x0044580c, FUN_0044580c, "regs(esi:ptr[0x41c]:u32[0x100..0x2000]) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_00418c68+ring_008943a0")
