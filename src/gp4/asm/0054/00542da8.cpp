#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00542da8
// edx selects a group of nine consecutive entries of the pointer table behind 0x0054d1ad (start
// index 0x500 + 9 * edx; edx = 0 or >= 0xe uses 0x5b4 instead when the word [0x7ad8ac] is nonzero).
// Entries are tried in order until the string an entry points at is non-empty; esi ends as that
// pointer, or as index + 1 of the last entry when all nine were empty. ebx = the dword at
// 0x8313cb + edx * 4 (edi = its address), ebp = ebp | ebx, ecx = the entries left, eax = the last
// tried index, edx = the high half of 9 * edx (unchanged on the 0x5b4 path). The flags are those of
// the final `or ebp, ebx`.
void __cdecl FUN_00542da8(gp4::Regs* r) {
    uint32_t edx = r->edx;
    const uint32_t entry = (edx << 2) + 0x008313cbu;
    const uint32_t loaded = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry));

    uint32_t esi;
    uint32_t eax = r->eax;
    if ((edx == 0u || edx >= 0xeu) && GP4_GLOBAL(uint16_t, 0x007ad8ac) != 0u) {
        esi = 0x5b4u;
    } else {
        const uint64_t product = static_cast<uint64_t>(9u) * edx;
        eax = static_cast<uint32_t>(product) + 0x500u;
        esi = eax;
        edx = static_cast<uint32_t>(product >> 32);
    }

    uint32_t ecx = 9u;
    do {
        eax = esi;
        gp4::Regs c = {};
        c.esi = esi;
        gp4::call_regs(0x0054d1ad, c);
        esi = c.esi;
        if (*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi)) != 0u) {
            break;
        }
        esi = eax + 1u;
        --ecx;
    } while (ecx != 0u);

    const uint32_t combined = r->ebp | loaded;
    r->eax = eax;
    r->ebx = loaded;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = entry;
    r->ebp = combined;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(combined));
}
GP4_IMPL(0x00542da8, FUN_00542da8, "regs(edx:u32[0..20], ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=strtab_00542da8")
