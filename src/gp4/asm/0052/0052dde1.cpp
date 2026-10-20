#include <gp4/gp4.h>
#include <gp4/flags.h>

// Key-name formatter: eax = key code (low word virtual key | 0x2100 modifier/extended bits),
// edi = output cursor. Translates the code to a scan code with MapVirtualKeyA(code, 0) (thunk
// 0x005a496c), builds the GetKeyNameText lParam ((scan | (code & 0x2100)) << 16 | 1), writes the
// name (up to 0x20 chars) at edi through thunk 0x005a4966 and advances edi by the returned length.
// ebx/ecx/edx/esi are preserved; flags are those of the final `add edi, eax`.
// FUNCTION: GP4 0x0052dde1
void __cdecl FUN_0052dde1(gp4::Regs* r) {
    const uint32_t code = r->eax;
    char* const cursor = (char*)(uintptr_t)r->edi;
    const uint32_t modifiers = code & 0x2100u;

    // 0x005a496c: jmp [0x5f0310] (user32!MapVirtualKeyA)
    const uint32_t scan = gp4::stdcall_<0x005a496c, uint32_t>(code, 0u);
    const uint32_t lparam = ((scan | modifiers) << 16) | 1u;
    // 0x005a4966: jmp [0x5f0314] (user32!GetKeyNameTextA)
    const uint32_t len = gp4::stdcall_<0x005a4966, uint32_t>(lparam, cursor, 0x20u);

    r->eax = len;
    r->edi = r->edi + len;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32((uint32_t)(uintptr_t)cursor, len));
}
GP4_IMPL(0x0052dde1, FUN_0052dde1, "regs(eax:u32, edi:ptr[0x40]) -> (eax, edi, cf, pf, af, zf, sf, of)")
