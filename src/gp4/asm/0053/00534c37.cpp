#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bit test of the mask [0x828164]: bit ((ebx[1] * 6 + ebx[2]) & 31). Clear: [0x828293] = ebx[3] (flags of the
// `test`, i.e. ZF/PF set); set: [0x828293] += 1 (flags of that add). eax is the bit mask in the set case,
// ebx[3] otherwise; ecx = ebx[1] * 6 + ebx[2].
// FUNCTION: GP4 0x00534c37
void __cdecl FUN_00534c37(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(r->ebx);
    const uint32_t bitIndex = GP4_FIELD(uint32_t, rec, 4) * 6u + GP4_FIELD(uint32_t, rec, 8);
    const uint32_t mask = 1u << (bitIndex & 31u);
    r->ecx = bitIndex;

    if ((GP4_GLOBAL(uint32_t, 0x00828164) & mask) == 0u) {
        const uint32_t value = GP4_FIELD(uint32_t, rec, 0xc);
        GP4_GLOBAL(uint32_t, 0x00828293) = value;
        r->eax = value;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
    } else {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
        GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
        r->eax = mask;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 1u));
    }
}
GP4_IMPL(0x00534c37, FUN_00534c37, "regs(ebx:ptr[0x10]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
