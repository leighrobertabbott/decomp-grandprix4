#include <gp4/gp4.h>
#include <gp4/flags.h>

// Searches the three device records of 0x3c bytes at 0x00827160 (seven key dwords each at +0x14..+0x2c)
// for the key in eax, row by row. Found: CF = 0, ebx = the record, ecx = its row (0..2), edx = the column
// (0..6); the other flags are those of the matching `cmp` (equal). Not found: CF = 1, ebx = the address
// past the third record, ecx = 3, edx = 0, the other flags those of `cmp ecx, 3` (equal).
// FUNCTION: GP4 0x005313bd
void __cdecl FUN_005313bd(gp4::Regs* r) {
    const uint32_t key = r->eax;
    uint32_t ebx = 0x00827160u;
    uint32_t ecx = 0u;
    uint32_t edx = 0u;
    bool found = false;

    do {
        if (key == GP4_FIELD(uint32_t, ebx, 0x14 + edx * 4u)) {
            found = true;
            break;
        }
        ++edx;
        if (edx == 7u) {
            ebx += 0x3cu;
            ++ecx;
            edx = 0u;
        }
    } while (ecx != 3u);

    const uint32_t arith = found ? (gp4::flags::sub32(key, key) & ~gp4::flags::CF)
                                 : (gp4::flags::sub32(3u, 3u) | gp4::flags::CF);
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005313bd, FUN_005313bd, "regs(eax:u32[0..8]) -> (ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=sel_00531859")
