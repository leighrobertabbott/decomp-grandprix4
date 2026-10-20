#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks (or dword +0x48, 0x20000) the track-ring records around every 0x160-byte record in [edi, ebp)
// whose words +0x8e / +0x90 are not both zero: eight records forward from it (the ring advance: +0x160;
// at or past the lap end 0x866ecc, below the pit start 0x891500 subtract the wrap 0x866ec4, at or past
// the pit end 0x866ef8 clamp to pit end - 0x160) and then eight records backward from it (below the pit
// start step back and wrap +0x866ec4 below the lap start 0x891504; from the pit start on step back and
// clamp at the pit start). A do-while: the walk ends when edi == ebp, so the last flag writer is
// `cmp edi, ebp` with equal operands. ebx and ecx are what the last marked walk left (ebx its final
// record pointer, ecx 0) or the entry values; ax is the last or-ed pair of words.
// FUNCTION: GP4 0x005523f2
void __cdecl FUN_005523f2(gp4::Regs* r) {
    uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t words;
    do {
        words = static_cast<uint32_t>(GP4_FIELD(uint16_t, edi, 0x8e)) | GP4_FIELD(uint16_t, edi, 0x90);
        if (words != 0u) {
            ebx = edi;
            for (ecx = 8u; ecx != 0u; --ecx) {
                GP4_FIELD(uint32_t, ebx, 0x48) |= 0x20000u;
                ebx += 0x160u;
                if (ebx >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    if (ebx < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        ebx -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                    } else if (ebx >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                        ebx = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                    }
                }
            }
            ebx = edi;
            for (ecx = 8u; ecx != 0u; --ecx) {
                if (ebx < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    ebx -= 0x160u;
                    if (ebx < GP4_GLOBAL(uint32_t, 0x00891504)) {
                        ebx += GP4_GLOBAL(uint32_t, 0x00866ec4);
                    }
                } else {
                    ebx -= 0x160u;
                    if (ebx < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        ebx = GP4_GLOBAL(uint32_t, 0x00891500);
                    }
                }
                GP4_FIELD(uint32_t, ebx, 0x48) |= 0x20000u;
            }
        }
        edi += 0x160u;
    } while (edi != ebp);
    r->eax = (r->eax & 0xffff0000u) | words;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(edi, ebp));
}
GP4_IMPL(0x005523f2, FUN_005523f2, "regs(eax:u32, ebx:u32, ecx:u32, edi:rec[0x8943a0,0x160,0..20], ebp:rel[edi,0x160,1..6]) -> (eax, ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_005523f2")
