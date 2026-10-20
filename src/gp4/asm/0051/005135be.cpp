#include <gp4/gp4.h>
#include <gp4/flags.h>

// Restores the 0x16254-byte state block 0x007ad890..0x007c3ae4 from the buffer at esi with `movsb`
// (direction flag clear), then copies the first word of the block to 0x007a8e02 and overwrites it with
// 0x53. Leaves esi one past the source, edi = edx = 0x007c3ae4 and the flags of the final
// `cmp edi, edx` (equal operands).
// FUNCTION: GP4 0x005135be
void __cdecl FUN_005135be(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x007ad890u));
    const uint8_t* const end = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x007c3ae4u));
    do {
        *dst++ = *src++;
    } while (dst != end);

    GP4_GLOBAL(uint16_t, 0x007a8e02) = GP4_GLOBAL(uint16_t, 0x007ad890);
    GP4_GLOBAL(uint16_t, 0x007ad890) = 0x53u;

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = 0x007c3ae4u;
    r->edx = 0x007c3ae4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x007c3ae4u, 0x007c3ae4u));
}
GP4_IMPL(0x005135be, FUN_005135be, "regs(esi:ptr[0x16260]:bytes) -> (esi, edi, edx, cf, pf, af, zf, sf, of); wall=1800")
