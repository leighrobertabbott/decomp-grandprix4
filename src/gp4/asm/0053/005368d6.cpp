#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies a path string through the script interpreter 0x0054d7c0 (esi = source, edi = destination): the
// signed byte comparison al > [0x008339ae] selects "D:\grand prix 3", otherwise "C:\". esi is saved and
// restored (push/pop); the other registers and the flags are what the interpreter leaves.
// FUNCTION: GP4 0x005368d6
void __cdecl FUN_005368d6(gp4::Regs* r) {
    const int32_t selector = static_cast<int8_t>(r->eax & 0xffu);
    const int32_t limit = static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x008339ae));
    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(r->eax, GP4_GLOBAL(uint8_t, 0x008339ae)));
    if (selector > limit) {
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0081e5d9)));
    } else {
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0081e6ea)));
    }
    gp4::call_regs(0x0054d7c0, c);
    c.esi = r->esi;
    *r = c;
}
GP4_IMPL(0x005368d6, FUN_005368d6, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
