#include <gp4/gp4.h>
#include <gp4/flags.h>

// Follows the pointer cell 0x0083e782 ten hops (esi = [esi]) to a record, and later three more hops
// from it to a second record. For each record the byte addressed by the pointer at +0xe selects
// one of two three-dword tables (bit 0x80 set: the second one); the cursor dword at +0x26 is reset
// to the table start unless it already lies 0..0xc bytes inside the table (signed). The first
// record uses 0x0081e3d8 / 0x0081e3e8, the second 0x0081e3f8 / 0x0081e408. pushal/popal keep every
// register; the flags are those of the last compare of the second record (`cmp eax, 0` when the
// cursor was below the table start, else `cmp eax, 0xc`).

static inline uint32_t LoadLink(uint32_t addr) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(addr));
}

static inline uint32_t ResetCursor(uint32_t node, uint32_t tableIfClear, uint32_t tableIfSet) {
    const uint8_t* const flagByte =
        reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(LoadLink(node + 0xeu)));
    const uint32_t table = (*flagByte & 0x80u) != 0u ? tableIfSet : tableIfClear;
    const int32_t offset = static_cast<int32_t>(LoadLink(node + 0x26u) - table);
    uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(offset), 0u);
    if (offset >= 0) {
        flags = gp4::flags::sub32(static_cast<uint32_t>(offset), 0xcu);
        if (offset <= 0xc) {
            return flags;
        }
    }
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(node + 0x26u)) = table;
    return flags;
}

// FUNCTION: GP4 0x0052d57f
void __cdecl FUN_0052d57f(gp4::Regs* r) {
    uint32_t link = 0x0083e782u;
    for (int hop = 0; hop < 10; ++hop) {
        link = LoadLink(link);
    }
    ResetCursor(link, 0x0081e3d8u, 0x0081e3e8u);

    link = LoadLink(LoadLink(LoadLink(link)));
    const uint32_t flags = ResetCursor(link, 0x0081e3f8u, 0x0081e408u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052d57f, FUN_0052d57f, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=menu_0052d57f")
