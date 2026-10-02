#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t flags = parityFlag(result);
    if (result < left) flags |= 0x001u;
    if (((left ^ right ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t flags = parityFlag(result);
    if (left < right) flags |= 0x001u;
    if (((left ^ right ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// FUNCTION: GP4 0x00402f9b
void __cdecl FUN_00402f9b(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    if ((GP4_FIELD(uint8_t, record, 0xe5) & 1u) != 0u) {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x1c0)));
        r->ecx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x1c2)));
        const uint32_t scaled = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x1be))) << 6;
        const uint32_t offset = GP4_FIELD(uint32_t, record, 0x08);
        r->edx = scaled + offset;
        flags = addFlags(scaled, offset);
    } else {
        gp4::call_regs(0x00402f8a, *r);
        const uint32_t before = r->edx;
        r->edx = before - 0x7200u;
        flags = subFlags(before, 0x7200u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00402f9b, FUN_00402f9b, "regs(esi:ptr[0x1ca]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
