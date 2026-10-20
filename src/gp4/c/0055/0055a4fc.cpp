#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the low byte of each of [0x0088f5d0] input words (read at esi; a word other than 0x1f is
// followed by a second, unused word) to the byte table 0x0088f5ca, then copies the last byte
// written to 0x0088f5c9. Leaves ebx = count, esi past the words, ax/cx/dx as the loop left them
// and the flags of the last "sub cx, 1".
// FUNCTION: GP4 0x0055a4fc
void __cdecl FUN_0055a4fc(gp4::Regs* r) {
    const uint16_t count = GP4_GLOBAL(uint16_t, 0x0088f5d0);
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t esi = r->esi;
    uint32_t ebx = 0u;
    uint16_t cx = count;
    uint32_t arith = gp4::flags::logic16(cx);

    if (cx != 0u) {
        uint16_t ax = 0u;
        uint16_t dx = static_cast<uint16_t>(edx);
        arith = gp4::flags::sub16(cx, 1u);
        cx = static_cast<uint16_t>(cx - 1u);
        do {
            ax = *reinterpret_cast<const uint16_t*>(esi);
            esi += 2u;
            GP4_ARRAY(uint8_t, 0x0088f5ca)[ebx] = static_cast<uint8_t>(ax);
            if (ax != 0x1fu) {
                dx = *reinterpret_cast<const uint16_t*>(esi);
                esi += 2u;
            }
            ebx += 1u;
            arith = gp4::flags::sub16(cx, 1u);
            cx = static_cast<uint16_t>(cx - 1u);
        } while ((cx & 0x8000u) == 0u);
        const uint8_t last = GP4_ARRAY(uint8_t, 0x0088f5c9)[ebx];
        GP4_GLOBAL(uint8_t, 0x0088f5c9) = last;
        eax = (eax & 0xffff0000u) | (static_cast<uint32_t>(ax) & 0xff00u) | last;
        edx = (edx & 0xffff0000u) | dx;
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = edx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055a4fc, FUN_0055a4fc, "regs(esi:ptr[0x100]:u16{0x1f,0x1f,0x20,0x1e,0,0x41,0xff1f}, eax:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=cnt_0055a4fc")
