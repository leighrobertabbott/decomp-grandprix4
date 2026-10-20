#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks the finishing positions of the drivers: after refreshing the number of places [0x00831a61] with
// 0x00548e66, entry k (1..count) of the order bytes 0x007c31dc names a driver ((byte & 0x3f) - 1); when that
// driver's time dword at 0x007c3208 is neither 0x80000000 nor <= -1000, the byte k is stored at
// 0x007c2f2d + [0x007ad894] + 0x11 * driver. Leaves eax/edx/esi (last driver), ebx (cursor), ecx (k+1),
// edi (store base) and the flags of the final `cmp ecx, [0x831a61]`.
// FUNCTION: GP4 0x00532396
void __cdecl FUN_00532396(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00548e66, c);

    const uint8_t* order = &GP4_ARRAY(uint8_t, 0x007c31dc)[0];
    uint32_t cursor = 0x007c31dcu;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x007ad894) + 0x007c2f2du;
    uint32_t k = 1u;
    uint32_t driver = 0u;
    uint32_t cell = 0u;
    uint32_t time = 0u;
    (void)order;
    do {
        driver = (GP4_ARRAY(uint8_t, 0x007c31dc)[k - 1u] & 0x3fu) - 1u;
        cell = driver * 4u + 0x007c3208u;
        time = GP4_GLOBAL(uint32_t, cell);
        if (time != 0x80000000u && static_cast<int32_t>(time) > -1000) {
            driver = driver * 0x11u;
            GP4_GLOBAL(uint8_t, driver + base) = static_cast<uint8_t>(k);
        }
        cursor++;
        k++;
    } while (k <= GP4_GLOBAL(uint32_t, 0x00831a61));

    const uint32_t flags = gp4::flags::sub32(k, GP4_GLOBAL(uint32_t, 0x00831a61));
    c.eax = time;
    c.ebx = cursor;
    c.ecx = k;
    c.edx = driver;
    c.esi = cell;
    c.edi = base;
    c.eflags = gp4::flags::merge(c.eflags, flags);
    *r = c;
}
GP4_IMPL(0x00532396, FUN_00532396, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+tabs_0043ced2+trk_007ad894+order_00532396")
