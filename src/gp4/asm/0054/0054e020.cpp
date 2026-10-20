#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inverse of 0x0054e045 for a text-edit record at edi (+0 = start of the text): walks the text from
// its start to the position given by dh = line and dl = column. With dh != 0 it first skips lines
// (every control byte below 0x20 other than NUL ends one; a NUL ends the walk at once, esi on the
// NUL); then it advances dl printable bytes, stopping early on a control byte or NUL. esi = the
// position reached, al = the last byte read, dh/dl = what is left of the counters (eax and edx
// keep their other bits). The flags are those of the last compare/test on the taken exit.
// FUNCTION: GP4 0x0054e020
void __cdecl FUN_0054e020(gp4::Regs* r) {
    uint32_t esi = GP4_FIELD(uint32_t, r->edi, 0);
    uint32_t al = r->eax & 0xffu;
    uint32_t dh = (r->edx >> 8) & 0xffu;
    uint32_t dl = r->edx & 0xffu;
    uint32_t flags = 0u;
    bool finished = false;

    if (dh != 0u) {
        for (;;) {
            al = GP4_GLOBAL(uint8_t, esi);
            flags = gp4::flags::logic8(al);
            if (al == 0u) {
                finished = true;
                break;
            }
            esi = esi + 1u;
            if (al >= 0x20u) {
                continue;
            }
            dh = (dh - 1u) & 0xffu;
            if (dh != 0u) {
                continue;
            }
            break;
        }
        if (!finished) {
            flags = gp4::flags::logic8(dl);
            if (dl == 0u) {
                finished = true;
            }
        }
    }

    if (!finished) {
        for (;;) {
            al = GP4_GLOBAL(uint8_t, esi);
            flags = gp4::flags::sub8(al, 0x20u);
            if (al < 0x20u) {
                break;
            }
            esi = esi + 1u;
            dl = (dl - 1u) & 0xffu;
            flags = gp4::flags::logic8(dl);
            if (dl == 0u) {
                break;
            }
        }
    }

    r->esi = esi;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->edx = (r->edx & 0xffff0000u) | (dh << 8) | dl;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e020, FUN_0054e020, "regs(eax:u32, edx:u32{0,1,2,3,5,0x108,0x100,0x101,0x102,0x103,0x105,0x200,0x201,0x202,0x203,0x300,0x301,0x302,0x305}, edi:ptr[0x10]:u32{=0xa20000,0xa20004,0xa20010,0xa20020,0xa20030}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")
