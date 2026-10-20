#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053657e
// `pushal ... popal`: the current file record [0x826d11] (its byte +0x55 is the directory mode) is passed to
// the directory setter 0x00536823 in al, then its first bytes (the file name) are opened with CreateFileA
// (GENERIC_READ, no sharing, no security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, no template) through the import
// thunk 0x005a48b8. A valid handle is closed again (thunk 0x005a48ee, called with the live registers so the
// flags it leaves are kept); registers are restored by the popal. Flags: on success CloseHandle's flags with
// CF cleared by clc; on INVALID_HANDLE_VALUE those of `cmp eax, -1` with CF set by stc.
void __cdecl FUN_0053657e(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00826d11)));
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | record[0x55];
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00536823, c);

    const uint32_t name = GP4_GLOBAL(uint32_t, 0x00826d11);
    const uint32_t handle = gp4::stdcall_<0x005a48b8, uint32_t>(name, 0x80000000u, 0u, 0u, 3u, 0x80u, 0u);
    uint32_t arith = gp4::flags::sub32(handle, 0xffffffffu);
    if (handle != 0xffffffffu) {
        gp4::Regs close = c;
        close.eax = handle;
        close.edx = name;
        close.eflags = gp4::flags::merge(close.eflags, arith);
        const uint32_t args[1] = {handle};
        gp4::call_regs_stdcall(0x005a48ee, close, args);
        r->eflags = gp4::flags::merge(r->eflags, close.eflags & ~gp4::flags::CF);
    } else {
        r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
    }
}
GP4_IMPL(0x0053657e, FUN_0053657e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rec_0052e42f")
