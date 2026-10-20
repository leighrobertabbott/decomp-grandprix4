#include <gp4/gp4.h>
#include <gp4/flags.h>

// ax = word 0x008911f6 - word [edi]; sets bit 27 (0x08000000) of dword [edi+0x48] when that
// difference is negative, else bit 28 (0x10000000). The flags are those of the final `or`.
// FUNCTION: GP4 0x005548b4
void __cdecl FUN_005548b4(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint16_t difference = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008911f6) - GP4_FIELD(uint16_t, record, 0));
    r->eax = (r->eax & 0xffff0000u) | difference;
    const uint32_t bit = (difference & 0x8000u) != 0u ? 0x08000000u : 0x10000000u;
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x48) | bit;
    GP4_FIELD(uint32_t, record, 0x48) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x005548b4, FUN_005548b4, "regs(eax:u32, edi:ptr[0x50]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
