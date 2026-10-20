#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005157b7
// Looks up three values from the record in esi: byte +0x7f indexes the byte table at 0x6f2c38
// (stored to [0x866098], also left in edi); byte +0x7c minus one, masked to 6 bits and clamped to
// 0x15, selects an entry of 0x2c bytes at 0x66d962 plus a byte offset taken through the two byte
// tables 0x66d936 and 0x6f2c6a (address stored to [0x865fd0]); [0x865fcc] receives the byte at
// 0x7c2ea9. Leaves eax = that byte, ebx = index * 0x2c, edx = the 0x6f2c6a offset, edi = the first
// lookup. The flags are those of `imul ebx, ebx, 0x2c` (index <= 0x15 never overflows: CF = OF = 0,
// the rest follow the result).
void __cdecl FUN_005157b7(gp4::Regs* r) {
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t first = GP4_ARRAY(uint8_t, 0x006f2c38)[record[0x7f]];
    GP4_GLOBAL(uint32_t, 0x00866098) = first;

    uint32_t index = static_cast<uint8_t>(record[0x7c] - 1u) & 0x3fu;
    if (static_cast<int32_t>(index) > 0x15) {
        index = 0x15u;
    }
    const uint32_t entryOffset = index * 0x2cu;
    const uint32_t group = GP4_ARRAY(uint8_t, 0x0066d936)[index];
    const uint32_t within = GP4_ARRAY(uint8_t, 0x006f2c6a)[group];
    GP4_GLOBAL(uint32_t, 0x00865fd0) =
        within + entryOffset + static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0066d962)));

    const uint32_t last = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    GP4_GLOBAL(uint32_t, 0x00865fcc) = last;

    r->eax = last;
    r->ebx = entryOffset;
    r->edx = within;
    r->edi = first;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(entryOffset));
}
GP4_IMPL(0x005157b7, FUN_005157b7, "regs(esi:ptr[0x88]:bytes) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
