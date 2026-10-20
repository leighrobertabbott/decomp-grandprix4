#include <gp4/gp4.h>
#include <gp4/flags.h>

// The two list-head setups take identical register arguments; kept as separate calls so the compiler
// cannot merge them into one call through a computed address.
static __declspec(noinline) void setupTwice(gp4::Regs& c) {
    gp4::call_regs(0x00534116, c);
}
static __declspec(noinline) void setupOnce(gp4::Regs& c) {
    gp4::call_regs(0x00534123, c);
}

// FUNCTION: GP4 0x00534c61
// Item setup from the record ebx: after 0x0053414a refills the eight-dword table 0x00828214, esi = [ebx+4]
// and eax = [ebx+8] go to the list-head setup 0x00534116 (twice, raising both) when [ebx+0xc] is 2, else to
// 0x00534123 once; then the (unaligned) counter at 0x00828293 is incremented. ecx and edi come from
// 0x0053414a, eax and esi from the setup; the flags are those of the final `add dword [0x828293], 1`.
void __cdecl FUN_00534c61(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053414a, c);

    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    c.esi = GP4_FIELD(uint32_t, rec, 4);
    c.eax = GP4_FIELD(uint32_t, rec, 8);
    const uint32_t kind = GP4_FIELD(uint32_t, rec, 0xc);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(kind, 2u));
    if (kind == 2u) {
        setupTwice(c);
    } else {
        setupOnce(c);
    }

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
    GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 1u));
}
GP4_IMPL(0x00534c61, FUN_00534c61, "regs(ebx:ptr[0x10]:u32{=0,1,2,3,5,8}) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=head_00852837+tab_00835846")
