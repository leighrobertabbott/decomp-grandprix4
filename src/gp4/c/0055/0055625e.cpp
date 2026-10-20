#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 0x160-byte ring records from [0x866ec0] up to (excluding) [0x866eb8] (do-while: the
// pointer is compared with the end after each step). In each record, every one of the dwords
// +0x2c, +0x3c, +0x30, +0x34, +0x38, +0x40 that has bit 0x20 set gets bit 0x400000, then every one
// with bit 0x10 set gets bit 0x200000. pushal/popal keep every register; the flags are those of
// the final `cmp edi, [0x866eb8]` (equal).

static inline void CopyBit(uint32_t record, uint32_t offset, uint32_t test, uint32_t set) {
    uint32_t& word = *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(record + offset));
    if ((word & test) != 0u) {
        word |= set;
    }
}

// FUNCTION: GP4 0x0055625e
void __cdecl FUN_0055625e(gp4::Regs* r) {
    uint32_t record = GP4_GLOBAL(uint32_t, 0x00866ec0);
    uint32_t end;
    do {
        CopyBit(record, 0x2cu, 0x20u, 0x400000u);
        CopyBit(record, 0x3cu, 0x20u, 0x400000u);
        CopyBit(record, 0x30u, 0x20u, 0x400000u);
        CopyBit(record, 0x34u, 0x20u, 0x400000u);
        CopyBit(record, 0x38u, 0x20u, 0x400000u);
        CopyBit(record, 0x40u, 0x20u, 0x400000u);
        CopyBit(record, 0x2cu, 0x10u, 0x200000u);
        CopyBit(record, 0x3cu, 0x10u, 0x200000u);
        CopyBit(record, 0x30u, 0x10u, 0x200000u);
        CopyBit(record, 0x34u, 0x10u, 0x200000u);
        CopyBit(record, 0x38u, 0x10u, 0x200000u);
        CopyBit(record, 0x40u, 0x10u, 0x200000u);
        record += 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866eb8);
    } while (record != end);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, end));
}
GP4_IMPL(0x0055625e, FUN_0055625e, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=bits_0055616f")
