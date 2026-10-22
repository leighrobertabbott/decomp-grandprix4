#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes a "name = value" style row at the cursor edi: the text at esi padded to 0x20 columns
// (0x0054c6f6), the separator text at 0x00833b16 ("=    "), the text at 0x00833b48, the text whose
// pointer arrived in eax, the text at 0x00833b48 again and finally the text at 0x00833b13, each
// expanded by 0x0054d7c0. eax is saved on the stack around the padded write. The registers are
// threaded through the callees; esi, edi and the flags are those the last expansion leaves.
static __declspec(noinline) void Expand(gp4::Regs& c, uint32_t text) {
    c.esi = text;
    gp4::call_regs(0x0054d7c0, c);
}

// FUNCTION: GP4 0x0054c6c1
void __cdecl FUN_0054c6c1(gp4::Regs* r) {
    const uint32_t value = r->eax;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0054c6f6, c);
    Expand(c, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b16))));
    Expand(c, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b48))));
    Expand(c, value);
    Expand(c, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b48))));
    Expand(c, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b13))));

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c6c1, FUN_0054c6c1, "regs(eax:ptr[0x40]:u8{=0,0,0,0,0x41,0x61,0x20,0x30,0x39}, ebx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:u8{=0,0,0,0,0x41,0x61,0x20,0x30,0x39}, edi:ptr[0x200]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
