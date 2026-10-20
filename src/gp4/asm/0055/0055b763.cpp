#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine on the record in edi: for each set bit k (0..7) of the mask byte [edi+0x67] it
// derives a byte from the word [edi+0x1a] minus the first byte of a 10-byte entry of the table at
// 0x00961b9c (selected by the word table 0x009559f8 at 48*word[edi+0x56] + (k < 4 ? 0xb : 0x14) + k),
// adjusted by 0x0055b74b, and sets bit k, k+8, k+16, k+24 of the result when its low 2, 3, 4, 5 bits are zero.
// The ORed result goes to dword [edi+0xa0]. Leaves esi = result, ebx = 0, ecx = 8, ebp / edx / eax from the
// last set bit (unchanged when none), and the flags of the final `cmp ecx, 8`.
// FUNCTION: GP4 0x0055b763
void __cdecl FUN_0055b763(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t bits = rec[0x67];
    uint32_t result = 0u;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    for (uint32_t k = 0u; k < 8u; ++k) {
        const bool isSet = (bits & 1u) != 0u;
        bits >>= 1;
        if (!isSet) continue;

        uint32_t index = static_cast<uint32_t>(GP4_FIELD(uint16_t, rec, 0x56)) * 3u;
        index <<= 4;
        index += (static_cast<int32_t>(k) < 4) ? 0xbu : 0x14u;
        index += k;
        const uint32_t entryNo = GP4_ARRAY(uint16_t, 0x009559f8)[index];
        uint8_t* const entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(entryNo * 10u + 0x00961b9cu));
        eax = (index & 0xffff0000u) | GP4_FIELD(uint16_t, rec, 0x1a);
        eax = (eax & 0xffffff00u) | static_cast<uint8_t>(eax - entry[8]);

        gp4::Regs c = {};
        c.eax = eax;
        c.ecx = k;
        c.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        gp4::call_regs(0x0055b74b, c);
        eax = c.eax;

        edx = 0u;
        ebp = 1u << k;
        if ((eax & 3u) == 0u) edx |= ebp;
        ebp <<= 8;
        if ((eax & 7u) == 0u) edx |= ebp;
        ebp <<= 8;
        if ((eax & 0xfu) == 0u) edx |= ebp;
        ebp <<= 8;
        if ((eax & 0x1fu) == 0u) edx |= ebp;
        result |= edx;
    }

    GP4_FIELD(uint32_t, rec, 0xa0) = result;
    r->eax = eax;
    r->ebx = 0u;
    r->ecx = 8u;
    r->edx = edx;
    r->esi = result;
    r->ebp = ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(8u, 8u));
}
GP4_IMPL(0x0055b763, FUN_0055b763, "regs(eax:u32, edx:u32, ebp:u32, edi:ptr[0xa8]:u32{=0,1,2,3,0x10000,0x20000,0x30000,0x40000,0x80000,0x1000000,0x3000000,0x10000000}) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of)")
