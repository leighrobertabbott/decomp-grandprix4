#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the saved-state block 0x007ad890..0x007c3ae4 (0x16254 bytes) to the buffer at edi with
// `movsb` (direction flag clear). Leaves edi one past the copy, esi = edx = 0x007c3ae4 and the flags of
// the final `cmp esi, edx` (equal operands).
// FUNCTION: GP4 0x00512c7d
void __cdecl FUN_00512c7d(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x007ad890u));
    const uint8_t* const end = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x007c3ae4u));
    do {
        *dst++ = *src++;
    } while (src != end);
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->esi = 0x007c3ae4u;
    r->edx = 0x007c3ae4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x007c3ae4u, 0x007c3ae4u));
}
GP4_IMPL(0x00512c7d, FUN_00512c7d, "regs(edi:ptr[0x16260]:bytes) -> (edi, esi, edx, cf, pf, af, zf, sf, of); globals=layout=block_00512c7d; maxinsn=400000; wall=1800")
