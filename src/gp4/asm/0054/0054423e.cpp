#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the nine position-adjusted captions of option group edx. [0x008313cb + edx * 4] holds nine 2-bit slot
// codes (ebx; slot k is bits 2k..2k+1, 0 = unused) and ebp the nine matching 2-bit "already drawn" masks. The
// text ids are the nine consecutive ids from 0x500 + 9 * edx (0x5b4 when edx is 0 or above 0xd and the word
// [0x007ad8ac] is nonzero). Slot k is drawn when its code is nonzero, ebp & ebx has no bit in the field and the
// position dword [0x008313a7 + k * 4] is not -1: code 1 subtracts [0x00b1bf80] from it, code 3 half of it; the
// result becomes the text y [0x00831564], [0x00831547] - 4 the text x [0x00831560], and 0x00546173 draws the id
// in esi (all inside pushal/popal). Exit: ebx = the dword of the group, ebp = ebp | ebx (flags of that `or`),
// ecx = 0, esi = the first id + 9, edi = 0x008313cb, eax = what the last slot left (0 unused, ebp & ebx & 3, -1
// or the adjusted position) and edx = the last nonzero code (the high half of 9 * edx before the first).
// FUNCTION: GP4 0x0054423e
void __cdecl FUN_0054423e(gp4::Regs* r) {
    const uint32_t group = r->edx;
    const uint32_t entry = (group << 2) + 0x008313cbu;
    const uint32_t codes = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry));
    const uint32_t drawn = r->ebp;

    uint32_t edx = group;
    uint32_t esi;
    if ((group == 0u || group >= 0xeu) && GP4_GLOBAL(uint16_t, 0x007ad8ac) != 0u) {
        esi = 0x5b4u;
    } else {
        const uint64_t product = static_cast<uint64_t>(9u) * group;
        esi = static_cast<uint32_t>(product) + 0x500u;
        edx = static_cast<uint32_t>(product >> 32);
    }

    const uint32_t* position = GP4_ARRAY(uint32_t, 0x008313a7);
    uint32_t eax = 0u;
    for (uint32_t k = 0u; k < 9u; ++k) {
        const uint32_t code = (codes >> (2u * k)) & 3u;
        eax = code;
        if (code != 0u) {
            edx = code;
            eax = (drawn >> (2u * k)) & (codes >> (2u * k)) & 3u;
            if (eax == 0u) {
                eax = position[k];
                if (eax != 0xffffffffu) {
                    if (code == 1u) {
                        eax = eax - GP4_GLOBAL(uint32_t, 0x00b1bf80);
                    } else if (code == 3u) {
                        eax = eax - (GP4_GLOBAL(uint32_t, 0x00b1bf80) >> 1);
                    }
                    gp4::stdcall_<0x00546327, void>(static_cast<int32_t>(eax));
                    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00831547) - 4u;
                    gp4::stdcall_<0x00546311, void>(static_cast<int32_t>(cursor));

                    gp4::Regs c = *r;
                    c.st_in = 0;
                    c.st_out = 0;
                    c.st_xin_mask = 0;
                    c.st_xout_mask = 0;
                    c.eax = eax;
                    c.ebx = cursor;
                    c.ecx = 9u - k;
                    c.edx = code;
                    c.edi = entry;
                    c.esi = esi + k;
                    gp4::call_regs(0x00546173, c);
                }
            }
        }
    }

    const uint32_t combined = drawn | codes;
    r->eax = eax;
    r->ebx = codes;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = esi + 9u;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(position)) + 36u;
    r->ebp = combined;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(combined));
}
GP4_IMPL(0x0054423e, FUN_0054423e, "regs(edx:u32[0..20], ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=strtab_00542da8+cursor_00545f09+surf_0054654a+pos_0054423e")
