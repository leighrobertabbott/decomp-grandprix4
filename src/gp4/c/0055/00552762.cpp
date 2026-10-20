#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first free entry of the 0x140-entry table (0xe0 bytes each) at 0x00854460; an entry is
// in use when bit 0 of its first dword is set. Returns eax = the index and edx = index * 0xe0, with
// CF clear when a free entry was found and CF set (index 0x140) when all are in use. The other flags
// are those of the last `test`/`cmp` (ZF and PF set in both cases).
// FUNCTION: GP4 0x00552762
void __cdecl FUN_00552762(gp4::Regs* r) {
    uint32_t index = 0u;
    uint32_t offset = 0u;
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x00854460);
    uint32_t flags;
    for (;;) {
        const uint32_t used = GP4_FIELD(uint32_t, rec, 0) & 1u;
        flags = gp4::flags::logic32(used);
        if (used == 0u) {
            break;
        }
        rec += 0xe0;
        offset += 0xe0u;
        ++index;
        flags = gp4::flags::sub32(index, 0x140u);
        if (index == 0x140u) {
            flags |= gp4::flags::CF;
            break;
        }
    }
    r->eax = index;
    r->edx = offset;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00552762, FUN_00552762, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=tbl_00854460")
