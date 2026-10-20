#include <gp4/gp4.h>
#include <gp4/flags.h>

// Steps the cursor esi over the item whose kind dword it points at: kind 1 skips 4 bytes, kind 2
// skips 8, kind 3 skips the stride held in the dword at 0x6975c4, any other kind leaves esi alone.
// eax is the kind. The flags are those of the last writer: the `add esi, ...` of kinds 1-3, else
// the `cmp eax, 3`.
// FUNCTION: GP4 0x005135fa
void __cdecl FUN_005135fa(gp4::Regs* r) {
    const uint32_t kind = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t cursor = r->esi;
    uint32_t arith;
    if (kind == 1u) {
        arith = gp4::flags::add32(cursor, 4u);
        cursor += 4u;
    } else if (kind == 2u) {
        arith = gp4::flags::add32(cursor, 8u);
        cursor += 8u;
    } else if (kind == 3u) {
        const uint32_t stride = GP4_GLOBAL(uint32_t, 0x006975c4);
        arith = gp4::flags::add32(cursor, stride);
        cursor += stride;
    } else {
        arith = gp4::flags::sub32(kind, 3u);
    }
    r->eax = kind;
    r->esi = cursor;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005135fa, FUN_005135fa, "regs(esi:ptr[0x40]:u32{=1,2,3,0,7}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
