#include <gp4/gp4.h>

struct Unknown_0043c6fb {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;        // record address inside the 0x160-byte record array
    uint8_t Unknown20[0xfd - 0x20];
    uint8_t UnknownFD;         // count, capped at 10
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043c6fb
// Remaining slots = max(0, 3 * min(UnknownFD, 10) - (record count past the boundary 0x891500)),
// result = remaining * (0xfa000 / 0x1280 = 216). edx = high dword of the product, ebp = 0x1280,
// edi = the (clamped) offset from the boundary, or the record address itself below it; ebx is
// preserved. The flags are those of the final one-operand "imul edx" (pinned oracle model:
// CF = OF = product does not fit 32 bits, AF = 0, PF/ZF/SF from the low dword).
void __cdecl FUN_0043c6fb(gp4::Regs* r) {
    const Unknown_0043c6fb* rec = reinterpret_cast<const Unknown_0043c6fb*>(static_cast<uintptr_t>(r->esi));

    uint32_t offset = rec->Unknown1C;
    uint32_t pastBoundary = 0;
    if (offset >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        offset -= GP4_GLOBAL(uint32_t, 0x00866f08);
        if (static_cast<int32_t>(offset) < 0) {
            offset = 0;
        }
        pastBoundary = offset;
    }
    const uint32_t records = pastBoundary / 0x160u;

    uint32_t level = rec->UnknownFD;
    if (level > 10u) {
        level = 10u;
    }
    int32_t remaining = static_cast<int32_t>(level * 3u - records);
    if (remaining < 0) {
        remaining = 0;
    }

    const int64_t product = static_cast<int64_t>(0xfa000 / 0x1280) * remaining;
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t overflow = (static_cast<int64_t>(static_cast<int32_t>(low)) != product) ? 0x801u : 0u;
    const uint32_t flags = overflow
        | parityFlag(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low >> 24) & 0x80u);

    r->eax = low;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = 0x1280u;
    r->edi = offset;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c6fb, FUN_0043c6fb, "regs(esi:ptr[0x105]:bytes) -> (eax, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
