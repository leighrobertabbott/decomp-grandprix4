#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00531859
// Looks the selector [0x7ad87c] (read through 0x530774) up in three rows of seven dwords: row k
// (at 0x827160 + 0x3c * k) holds the keys at +0x14..+0x2c; on the first equal key j the value at
// +0x28 + 4 * j of that row is returned, otherwise 0x1000. ebx, ecx and edx are saved and
// restored. Both exits leave the flags of an equal compare (`cmp eax, [..]` on a hit, `cmp ecx, 3`
// when the rows run out): ZF = PF = 1, the rest clear.
void __cdecl FUN_00531859(gp4::Regs* r) {
    const uint32_t key = gp4::cdecl_<0x00530774, uint32_t>();

    const uint8_t* row = GP4_ARRAY(uint8_t, 0x00827160);
    uint32_t result = 0x1000u;
    uint32_t rowIndex = 0u;
    uint32_t column = 0u;
    bool found = false;
    do {
        if (key == *reinterpret_cast<const uint32_t*>(row + column * 4u + 0x14u)) {
            result = *reinterpret_cast<const uint32_t*>(row + column * 4u + 0x28u);
            found = true;
            break;
        }
        ++column;
        if (column == 7u) {
            row += 0x3c;
            ++rowIndex;
            column = 0u;
        }
    } while (rowIndex != 3u);
    (void)found;

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(1u, 1u));
}
GP4_IMPL(0x00531859, FUN_00531859, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=sel_00531859")
