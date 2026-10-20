#include <gp4/gp4.h>
#include <gp4/flags.h>

// Restores the game-state block 0x0066d7a4..0x006975bc (171544 bytes) from the buffer at esi with
// `movsb` (direction flag clear), then calls the empty routine 0x005ae990. Leaves esi one past the
// source, edi = edx = 0x006975bc and the flags of the final `cmp edi, edx` (equal operands).
// FUNCTION: GP4 0x005135e5
void __cdecl FUN_005135e5(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0066d7a4u));
    const uint8_t* const end = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x006975bcu));
    do {
        *dst++ = *src++;
    } while (dst != end);

    gp4::cdecl_<0x005ae990, void>();

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = 0x006975bcu;
    r->edx = 0x006975bcu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x006975bcu, 0x006975bcu));
}
GP4_IMPL(0x005135e5, FUN_005135e5, "regs(esi:ptr[0x29e20]:bytes) -> (esi, edi, edx, cf, pf, af, zf, sf, of); maxinsn=700000; wall=1800")
