#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up every segment of the ring (records of 0x160 bytes from [0x866ec0] up to [0x866eb8]) in the
// table of 0x60-byte entries at 0x9559f8: record k's 0x60-byte key is read from 0x6f6dc4 + k * 0x60
// (esi). 0x00558de7 searches the table; when it finds nothing (CF set) 0x00558e19 appends the key.
// The resulting index (eax) is stored as the word +0x56 of the segment record. pushal/popal keep every
// register; the flags are those of the final `cmp edi, [0x866eb8]` (equal).
// FUNCTION: GP4 0x00558e4c
void __cdecl FUN_00558e4c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edi = GP4_GLOBAL(uint32_t, 0x00866ec0);
    c.esi = 0x006f6dc4u;
    uint32_t end;
    do {
        gp4::call_regs(0x00558de7, c);
        if ((c.eflags & 1u) != 0u) {
            gp4::call_regs(0x00558e19, c);
        }
        GP4_FIELD(uint16_t, c.edi, 0x56) = static_cast<uint16_t>(c.eax);
        c.esi += 0x60u;
        c.edi += 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866eb8);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edi, end));
    } while (c.edi != end);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00558e4c, FUN_00558e4c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=segs_00558d9b+recs_00558e4c")
