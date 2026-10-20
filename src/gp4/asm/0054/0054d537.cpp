#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint32_t Dword(uint32_t address) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
}

// jg: ZF = 0 and SF == OF.
static inline bool Jg(uint32_t f) {
    return (f & gp4::flags::ZF) == 0u && (((f & gp4::flags::SF) != 0u) == ((f & gp4::flags::OF) != 0u));
}

// jl: SF != OF.
static inline bool Jl(uint32_t f) {
    return ((f & gp4::flags::SF) != 0u) != ((f & gp4::flags::OF) != 0u);
}

// Skips `count` pixels of the 2-bits-per-pixel mask stream (esi, 16 pixels per dword, edx pixels
// left in ebx): `sub edx, count; jg` else reloads dwords until edx > 0 again (`add edx, 0x10; jle`)
// and takes count = 0x10 - edx; finally `shl eax, 1; xchg ecx, eax; shr ebx, cl; xchg ecx, eax`,
// which leaves eax = 2 * count and ebx shifted by (2 * count) & 31.
static inline void SkipMask(uint32_t count, uint32_t& eax, uint32_t& ebx, uint32_t& edx, uint32_t& esi) {
    uint32_t f = gp4::flags::sub32(edx, count);
    edx -= count;
    eax = count;
    if (!Jg(f)) {
        do {
            ebx = Dword(esi);
            esi += 4u;
            f = gp4::flags::add32(edx, 0x10u);
            edx += 0x10u;
        } while (!Jg(f));
        eax = 0x10u - edx;
    }
    eax <<= 1;
    ebx >>= (eax & 31u);
}

// FUNCTION: GP4 0x0054d537
// Clipped version of 0x0054d4ee: expands a 2-bits-per-pixel mask stream (esi) of [0x833f7c] rows by
// [0x833f78] pixels, placed at (eax, ebx) in the clip window rows [0x833d00, 0x833d04] and columns
// [0x833cf8, 0x833cfc], into the byte surface at edi (colour [0x833cd5] where bit 1 of the pixel is
// set, row pitch [0x822244], [0x82224c] bytes per skipped row). The clipped amounts go to
// 0x833d10 (rows above), 0x833d14 (below), 0x833d08 (columns left) and 0x833d0c (right); an empty
// intersection leaves at once. The skipped pixels are consumed from the mask stream. Then the empty
// routine 0x0054d6f0 is called; registers and flags are those of the last executed instruction
// of the chosen path (a `cmp` for an early exit, the `dec ecx` of the row loop otherwise).
void __cdecl FUN_0054d537(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t fl;

    do {
        fl = gp4::flags::sub32(eax, GP4_GLOBAL(uint32_t, 0x00833d04));
        if (Jg(fl)) {
            break;
        }
        ecx = eax;
        eax += GP4_GLOBAL(uint32_t, 0x00833f7c);
        eax -= 1u;
        fl = gp4::flags::sub32(eax, GP4_GLOBAL(uint32_t, 0x00833d00));
        if (Jl(fl)) {
            break;
        }
        fl = gp4::flags::sub32(eax, GP4_GLOBAL(uint32_t, 0x00833d04));
        eax -= GP4_GLOBAL(uint32_t, 0x00833d04);
        if (Jl(fl)) {
            eax = 0u;
        }
        GP4_GLOBAL(uint32_t, 0x00833d14) = eax;
        eax = GP4_GLOBAL(uint32_t, 0x00833d00);
        fl = gp4::flags::sub32(eax, ecx);
        eax -= ecx;
        if (Jl(fl)) {
            eax = 0u;
        }
        GP4_GLOBAL(uint32_t, 0x00833d10) = eax;

        fl = gp4::flags::sub32(ebx, GP4_GLOBAL(uint32_t, 0x00833cfc));
        if (Jg(fl)) {
            break;
        }
        eax = ebx + GP4_GLOBAL(uint32_t, 0x00833f78);
        eax -= 1u;
        fl = gp4::flags::sub32(eax, GP4_GLOBAL(uint32_t, 0x00833cf8));
        if (Jl(fl)) {
            break;
        }
        fl = gp4::flags::sub32(eax, GP4_GLOBAL(uint32_t, 0x00833cfc));
        eax -= GP4_GLOBAL(uint32_t, 0x00833cfc);
        if (!Jg(fl)) {
            eax = 0u;
        }
        GP4_GLOBAL(uint32_t, 0x00833d0c) = eax;
        eax = GP4_GLOBAL(uint32_t, 0x00833cf8);
        fl = gp4::flags::sub32(eax, ebx);
        eax -= ebx;
        if (!Jg(fl)) {
            eax = 0u;
        }
        GP4_GLOBAL(uint32_t, 0x00833d08) = eax;

        ecx = GP4_GLOBAL(uint32_t, 0x00833f7c);
        ecx -= GP4_GLOBAL(uint32_t, 0x00833d10);
        fl = gp4::flags::sub32(ecx, GP4_GLOBAL(uint32_t, 0x00833d14));
        ecx -= GP4_GLOBAL(uint32_t, 0x00833d14);
        if (!Jg(fl)) {
            break;
        }

        ebx = Dword(esi);
        esi += 4u;
        edx = 0x10u;
        eax = GP4_GLOBAL(uint32_t, 0x00833d10);
        if (eax != 0u) {
            eax *= GP4_GLOBAL(uint32_t, 0x0082224c);
            edi += eax;
            eax = GP4_GLOBAL(uint32_t, 0x00833d10);
            eax *= GP4_GLOBAL(uint32_t, 0x00833f78);
            SkipMask(eax, eax, ebx, edx, esi);
        }

        do {
            const uint32_t rows = ecx;
            ecx = GP4_GLOBAL(uint32_t, 0x00833f78);
            eax = GP4_GLOBAL(uint32_t, 0x00833d08);
            if (eax != 0u) {
                ecx -= eax;
                edi += eax;
                SkipMask(eax, eax, ebx, edx, esi);
            }
            ecx -= GP4_GLOBAL(uint32_t, 0x00833d0c);
            const uint8_t colour = GP4_GLOBAL(uint8_t, 0x00833cd5);
            eax = (eax & 0xffffff00u) | colour;
            do {
                const bool set = ((ebx >> 1) & 1u) != 0u;
                ebx >>= 2;
                if (set) {
                    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi)) = colour;
                }
                edi += 1u;
                edx -= 1u;
                if (edx == 0u) {
                    ebx = Dword(esi);
                    esi += 4u;
                    edx = 0x10u;
                }
                ecx -= 1u;
            } while (ecx != 0u);

            eax = GP4_GLOBAL(uint32_t, 0x00833d0c);
            if (eax != 0u) {
                edi += eax;
                SkipMask(eax, eax, ebx, edx, esi);
            }
            edi -= GP4_GLOBAL(uint32_t, 0x00833f78);
            const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x00822244);
            const uint32_t addFlags = gp4::flags::add32(edi, pitch);
            edi += pitch;
            ecx = rows;
            fl = gp4::flags::dec32(ecx, addFlags);
            ecx -= 1u;
        } while (ecx != 0u);
    } while (false);

    gp4::Regs c = {};
    gp4::call_regs(0x0054d6f0, c);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x0054d537, FUN_0054d537, "regs(eax:i32[-8..44], ebx:i32[-40..44], esi:ptr[0x400]:bytes, edi:ptr[0x2000]:bytes) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=clip_0054d537")
