#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-field click handler (runs only when bit 1 of 0x0082fd6a is set, which it clears on
// exit): with a current menu [0x0082fd34] it runs the mode dispatcher 0x0053d9a6 and, for the
// modes 2/3, either flags 0x00828103 and calls 0x0053da96 (no hit [0x0082fd88]) or removes the
// cursor glyph from the text buffer 0x0082811f (0x00533dc5), walks the buffer from the start
// subtracting each glyph width (font table [0x00833f90], first char [0x00833f48]) from the
// click offset [0x0082fd94] - [0x00828113] + [0x0082811b] to find the new cursor [0x008280ed]
// (clamped to [0x008280f5] - [0x008280f9]) and re-inserts the cursor byte [0x00828161]
// there (0x00533d9e). The final flags are those of `and byte [0x82fd6a], 0xfd`.

// FUNCTION: GP4 0x005339ed
void __cdecl FUN_005339ed(gp4::Regs* r) {
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    if ((gate & 2u) == 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(gate & 2u));
        return;
    }
    if (GP4_GLOBAL(uint32_t, 0x0082fd34) != 0) {
        gp4::call_regs(0x0053d9a6, *r);
        const uint32_t mode = GP4_GLOBAL(uint32_t, 0x0082fd9c);
        r->edx = mode;
        if (mode == 2u || mode == 3u) {
            if (GP4_GLOBAL(uint32_t, 0x0082fd88) == 0) {
                GP4_GLOBAL(uint8_t, 0x00828103) = 0xffu;
                gp4::cdecl_<0x0053da96, void>();
            } else {
                GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
                gp4::call_regs(0x00533dc5, *r);
                GP4_GLOBAL(uint32_t, 0x008280ed) = 0u;
                uint32_t esi = 0x0082811fu;
                GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
                uint32_t eax = r->eax;
                uint32_t ebx = r->ebx;
                const int32_t d0 = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0082fd94)
                                                        - GP4_GLOBAL(uint32_t, 0x00828113));
                const int32_t org = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0082811b));
                uint32_t edx = static_cast<uint32_t>(d0) + static_cast<uint32_t>(org);
                if (static_cast<int64_t>(d0) + static_cast<int64_t>(org) > 0) {
                    for (;;) {
                        const uint8_t c = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
                        ++esi;
                        eax = (eax & 0xffffff00u) | c;
                        if ((eax & 0xffffu) <= 0x13u) break;
                        GP4_GLOBAL(uint8_t, 0x00833f76) = c;
                        ebx = GP4_GLOBAL(uint32_t, 0x00833f90);
                        const uint32_t slot = ((static_cast<uint32_t>(c) - GP4_GLOBAL(uint32_t, 0x00833f48)) << 2) + ebx;
                        ebx += *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(slot));
                        eax = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ebx));
                        edx -= eax;
                        gp4::cdecl_<0x0054d6f1, void>();
                        edx -= ebx;
                        const int32_t rest = static_cast<int32_t>(edx);
                        const int32_t margin = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00833f6c));
                        edx = static_cast<uint32_t>(rest) - static_cast<uint32_t>(margin);
                        if (static_cast<int64_t>(rest) - static_cast<int64_t>(margin) <= 0) break;
                        eax = GP4_GLOBAL(uint8_t, 0x00833f76);
                        GP4_GLOBAL(uint8_t, 0x00833f75) = static_cast<uint8_t>(eax);
                        GP4_GLOBAL(uint32_t, 0x008280ed) += 1u;
                    }
                }
                eax = GP4_GLOBAL(uint32_t, 0x008280f5) - GP4_GLOBAL(uint32_t, 0x008280f9);
                if (static_cast<int32_t>(eax) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x008280ed))) {
                    GP4_GLOBAL(uint32_t, 0x008280ed) = eax;
                }
                eax = (eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
                r->eax = eax;
                r->ebx = ebx;
                r->edx = edx;
                r->esi = esi;
                gp4::call_regs(0x00533d9e, *r);
            }
        }
    }
    const uint8_t cleared = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0082fd6a) & 0xfdu);
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = cleared;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x005339ed, FUN_005339ed, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6+span_00533dc5+font_005339ed")
