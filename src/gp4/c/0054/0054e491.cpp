#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the table of NUL-terminated strings (count at 0x00838b91, pointers from 0x00838b99) and
// rewrites every byte in place: a space becomes '_' and the result is translated through the
// character table of 0x0054d755. The whole register file is saved (pushal/popal); the flags are
// those of the loop-closing `dec ecx` / the initial `or ecx, ecx`, i.e. a zero result with CF=0
// (the preceding `cmp byte ptr [edi], 0` never carries).
// (v2: same code, to read the verdict details)
// FUNCTION: GP4 0x0054e491
void __cdecl FUN_0054e491(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x00838b91);
    const uint32_t* table = GP4_ARRAY(const uint32_t, 0x00838b99);

    if (count != 0) {
        do {
            uint8_t* p = (uint8_t*)(uintptr_t)*table++;
            while (*p != 0) {
                uint32_t ch = *p;
                if (ch == 0x20u) ch = 0x5fu;
                gp4::Regs c = {};
                c.eax = ch;
                gp4::call_regs(0x0054d755, c);
                *p = (uint8_t)c.eax;
                p++;
            }
            count--;
        } while (count != 0);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0));
}
GP4_IMPL(0x0054e491, FUN_0054e491, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=strs_00838b91+xlat_00833e18")
