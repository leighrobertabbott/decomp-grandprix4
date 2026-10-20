#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005364c5
// Loads a whole file into a caller buffer: esi = file name, edi = destination buffer, ecx = buffer
// capacity (kept in 0x00829410 / 0x0082940c). CreateFileA(name, GENERIC_READ, 0, NULL,
// OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) through the thunk 0x005a48b8, each Win32 call is
// bracketed by push/pop of ebx, ecx and edx. The handle goes to 0x0081ea28 and the open flag
// 0x0081ea2c = 0xff; 0x0053644d fetches the size (eax, CF cleared). Failure codes stored in
// 0x00824566: 2 = open failed (-1), 3 = size call reported carry, 4 = size above the capacity
// (unsigned), 5 = ReadFile (thunk 0x005a48f4, count -> 0x0081ea38) did not return 1. Every
// failure closes the handle through 0x0053646a and ends with CF set. Success closes the handle
// (CloseHandle, thunk 0x005a48ee), clears the open flag and the failure code and returns CF clear
// with eax = the CloseHandle result.
void __cdecl FUN_005364c5(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082940c) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00829410) = r->ecx;

    const uint32_t ebx0 = r->ebx;
    const uint32_t ecx0 = r->ecx;
    const uint32_t edx0 = r->edx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t failure;

    const uint32_t openArgs[7] = {r->esi, 0x80000000u, 0u, 0u, 3u, 0x80u, 0u};
    gp4::call_regs_stdcall(0x005a48b8, c, openArgs);
    c.ebx = ebx0;
    c.ecx = ecx0;
    c.edx = edx0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0xffffffffu));
    if (c.eax == 0xffffffffu) {
        failure = 2;
    } else {
        GP4_GLOBAL(uint32_t, 0x0081ea28) = c.eax;
        GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0xff;
        gp4::call_regs(0x0053644d, c);
        if ((c.eflags & gp4::CF) != 0u) {
            failure = 3;
        } else {
            const uint32_t capacity = GP4_GLOBAL(uint32_t, 0x00829410);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, capacity));
            if (c.eax > capacity) {
                failure = 4;
            } else {
                const uint32_t readArgs[5] = {
                    GP4_GLOBAL(uint32_t, 0x0081ea28), GP4_GLOBAL(uint32_t, 0x0082940c), c.eax,
                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x0081ea38))), 0u};
                gp4::call_regs_stdcall(0x005a48f4, c, readArgs);
                c.ebx = ebx0;
                c.ecx = ecx0;
                c.edx = edx0;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
                if (c.eax != 1u) {
                    failure = 5;
                } else {
                    const uint32_t closeArgs[1] = {GP4_GLOBAL(uint32_t, 0x0081ea28)};
                    gp4::call_regs_stdcall(0x005a48ee, c, closeArgs);
                    GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0;
                    GP4_GLOBAL(uint8_t, 0x00824566) = 0;
                    r->eax = c.eax;
                    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::CF);
                    return;
                }
            }
        }
    }

    GP4_GLOBAL(uint8_t, 0x00824566) = failure;
    gp4::call_regs(0x0053646a, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags | gp4::CF);
}
GP4_IMPL(0x005364c5, FUN_005364c5, "regs(ecx:u32, edi:u32, esi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
