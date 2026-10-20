#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ; ... ; popal`: for each of the 17 entries (ebx = 0..16) of each of the 22 records
// (eax = 0..21) of the array at 0x007adb4c, clears bytes +0 and +3 of the entry through 0x0051f82c
// (which stores cl at [entry address + edx]). popal restores every general register; the flags stay
// those of the final `cmp ebx, 0x11` (equal).
// FUNCTION: GP4 0x005329e2
void __cdecl FUN_005329e2(gp4::Regs* r) {
    for (uint32_t entry = 0u; entry < 0x11u; ++entry) {
        uint32_t record = 0u;
        do {
            gp4::Regs first = *r;
            first.st_in = 0;
            first.st_out = 0;
            first.eax = record;
            first.ebx = entry;
            first.ecx = 0u;
            first.edx = 0u;
            gp4::call_regs(0x0051f82c, first);

            gp4::Regs second = *r;
            second.st_in = 0;
            second.st_out = 0;
            second.eax = record;
            second.ebx = entry;
            second.ecx = 0u;
            second.edx = 3u;
            gp4::call_regs(0x0051f82c, second);
            ++record;
        } while (record < 0x16u);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x11u, 0x11u));
}
GP4_IMPL(0x005329e2, FUN_005329e2, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=hdr_005329e2")
