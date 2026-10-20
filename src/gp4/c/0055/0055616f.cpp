#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 0x160-byte records of the ring from the one after [0x866ec0] up to (and including) the
// one just before [0x866eb8] (do-while: the pointer is compared with the end after each step). For
// each record the dword +0x2c is xored with the previous record's +0x2c; the changed bits are
// or-ed into the dwords +0x30, +0x34, +0x38, +0x40, +0x3c of the record (stored only when some
// changed bit is missing). pushal/popal keep every register; the flags are those of the final
// `cmp edi, [0x866eb8]` (equal: ZF and PF).

static inline void MergeBits(uint32_t record, uint32_t offset, uint32_t bits) {
    uint32_t& word = *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(record + offset));
    if ((word & bits) != bits) {
        word |= bits;
    }
}

// FUNCTION: GP4 0x0055616f
void __cdecl FUN_0055616f(gp4::Regs* r) {
    uint32_t record = GP4_GLOBAL(uint32_t, 0x00866ec0) + 0x160u;
    uint32_t end;
    do {
        const uint32_t changed =
            *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record - 0x134u)) ^
            *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record + 0x2cu));
        if (changed != 0u) {
            MergeBits(record, 0x30u, changed);
            MergeBits(record, 0x34u, changed);
            MergeBits(record, 0x38u, changed);
            MergeBits(record, 0x40u, changed);
            MergeBits(record, 0x3cu, changed);
        }
        record += 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866eb8);
    } while (record != end);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, end));
}
GP4_IMPL(0x0055616f, FUN_0055616f, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=bits_0055616f")
