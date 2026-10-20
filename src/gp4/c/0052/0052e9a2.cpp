#include <gp4/gp4.h>
#include <gp4/flags.h>

// Opens the file named at 0x0082471a and reads its 0x20-byte header. After the field reset
// 0x0052e976 (registers and flags kept) everything runs inside pushal/popal: a reserved device
// name (0x0052f578 sets CF) fails at once; otherwise the directory is reset to the drive root
// (0x005367a8) and CreateFileA(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
// NULL) runs through the thunk 0x005a48b8. A valid handle goes to [0x0081ea28] with the open flag
// [0x0081ea2c] = 0xff, the size routine 0x0053644d stores the size at [0x00826d21], GetFileTime
// (last write time -> 0x00826cba) and FileTimeToDosDateTime (date word 0x00826cc2, time word
// 0x00826cc6) give the year (+1980) at 0x00826d1a, month-1, day-1, hours, minutes and seconds
// (2 s units doubled) at 0x00826d1c..0x00826d20, [0x00826d15] = 0xff and ReadFile reads 0x20 bytes
// to 0x00826d25 (count at 0x0081ea38). Any failure stores 0x250 at [0x00826d16]. The file is then
// closed by 0x0053646a, whose flags (`cmp byte [0x81ea2c], 0`) are the ones returned.
// FUNCTION: GP4 0x0052e9a2
void __cdecl FUN_0052e9a2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0052e976, c);
    gp4::call_regs(0x0052f578, c);
    bool ok = false;
    if ((c.eflags & gp4::CF) == 0u) {
        gp4::call_regs(0x005367a8, c);
        const uint32_t name = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0082471a)));
        const uint32_t handle = gp4::stdcall_<0x005a48b8, uint32_t>(name, 0x80000000u, 0u, 0u, 3u, 0x80u, 0u);
        if (handle != 0xffffffffu) {
            GP4_GLOBAL(uint32_t, 0x0081ea28) = handle;
            GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0xffu;
            c.eax = handle;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(handle, 0xffffffffu));
            gp4::call_regs(0x0053644d, c);
            if ((c.eflags & gp4::CF) == 0u) {
                GP4_GLOBAL(uint32_t, 0x00826d21) = c.eax;
                const uint32_t stamp = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00826cba)));
                gp4::stdcall_<0x005a494e, uint32_t>(GP4_GLOBAL(uint32_t, 0x0081ea28), 0u, 0u, stamp);
                gp4::stdcall_<0x005a4918, uint32_t>(
                    stamp,
                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00826cc2))),
                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00826cc6))));
                const uint32_t date = GP4_GLOBAL(uint32_t, 0x00826cc2);
                const uint32_t time = GP4_GLOBAL(uint32_t, 0x00826cc6);
                GP4_GLOBAL(uint16_t, 0x00826d1a) = static_cast<uint16_t>(((date >> 9) & 0x7fu) + 0x7bcu);
                GP4_GLOBAL(uint8_t, 0x00826d1c) = static_cast<uint8_t>(((date >> 5) & 0xfu) - 1u);
                GP4_GLOBAL(uint8_t, 0x00826d1d) = static_cast<uint8_t>((date & 0x1fu) - 1u);
                GP4_GLOBAL(uint8_t, 0x00826d1e) = static_cast<uint8_t>((time >> 11) & 0x1fu);
                GP4_GLOBAL(uint8_t, 0x00826d1f) = static_cast<uint8_t>((time >> 5) & 0x3fu);
                GP4_GLOBAL(uint8_t, 0x00826d20) = static_cast<uint8_t>((time & 0x1fu) << 1);
                GP4_GLOBAL(uint8_t, 0x00826d15) = 0xffu;
                const uint32_t read = gp4::stdcall_<0x005a48f4, uint32_t>(
                    GP4_GLOBAL(uint32_t, 0x0081ea28),
                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00826d25))),
                    0x20u,
                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x0081ea38))),
                    0u);
                ok = read == 1u;
            }
        }
    }
    if (!ok) {
        GP4_GLOBAL(uint32_t, 0x00826d16) = 0x250u;
    }
    gp4::call_regs(0x0053646a, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052e9a2, FUN_0052e9a2, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=lower_00833e18+name_0082471a")
