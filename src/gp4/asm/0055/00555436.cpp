#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = ((sign-extended word [edi+0x60] + 0x50) * 8 * ([edi+0x150] >> 14)) >> 14, the low 32 bits of the
// 64-bit signed product shifted right by 14 (imul + shrd eax, edx, 14). ebx and edx are saved and
// restored. The flags are those of the shrd: CF = bit 13 of the product, ZF/SF/PF of eax, AF = 0 and
// OF = the sign change between the product shifted by 13 and by 14 (the pinned oracle model for counts > 1).
// FUNCTION: GP4 0x00555436
void __cdecl FUN_00555436(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t scale = static_cast<int32_t>(GP4_FIELD(uint32_t, record, 0x150)) >> 14;
    const int32_t length =
        (static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, record, 0x60))) + 0x50) * 8;

    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(length) * static_cast<int64_t>(scale));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(product >> 32);
    const uint32_t previous = (low >> 13) | (high << 19);
    const uint32_t result = (low >> 14) | (high << 18);

    uint32_t arith = gp4::flags::szp(result, 32);
    if ((low >> 13) & 1u) {
        arith |= gp4::flags::CF;
    }
    if (((previous ^ result) >> 31) & 1u) {
        arith |= gp4::flags::OF;
    }
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00555436, FUN_00555436, "regs(edi:ptr[0x158]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
