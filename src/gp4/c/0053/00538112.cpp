#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00538112
// `pushal ... popal`: when the file-mode byte [0x829ba3] is 2, makes 0x005367b9 (eax = 1) set the
// current directory, then CreateFileA("f1gptemp.tmp", GENERIC_READ | GENERIC_WRITE, 0, NULL,
// OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) through the import thunk 0x005a48b8. A valid handle is
// stored at [0x81ea28] and [0x81ea2c] is set to 0xff. Registers are restored by the popal; the flags
// are those of the last compare (`cmp byte [0x829ba3], 2` or `cmp eax, -1`) with CF forced by
// clc (success or other mode) / stc (INVALID_HANDLE_VALUE).
void __cdecl FUN_00538112(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829ba3);
    uint32_t arith = gp4::flags::sub8(mode, 2u);
    if (mode == 2u) {
        gp4::Regs c = *r;
        c.eax = 1u;
        gp4::call_regs(0x005367b9, c);
        const uint32_t handle = gp4::stdcall_<0x005a48b8, uint32_t>(
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x008297a0))),
            0xc0000000u, 0u, 0u, 3u, 0x80u, 0u);
        arith = gp4::flags::sub32(handle, 0xffffffffu);
        if (handle != 0xffffffffu) {
            GP4_GLOBAL(uint32_t, 0x0081ea28) = handle;
            GP4_GLOBAL(uint8_t, 0x0081ea2c) = 0xffu;
            arith &= ~gp4::flags::CF;
        } else {
            arith |= gp4::flags::CF;
        }
    } else {
        arith &= ~gp4::flags::CF;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00538112, FUN_00538112, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
