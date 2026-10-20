#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = menu value record (+0xd width selector, +0xe pointer to the value cell). Reads the value
// through 0x00539944 (ebx = [esi+0xe]) and passes it as the second argument of the stdcall routine
// 0x00503230 with the code 0x2d (the text built there is "%d%s" of the table entry at 0x7ad8e8 +
// 4 * value). Both calls sit inside pushal/popal pairs, so every register comes back unchanged and
// only the flags of the stdcall callee remain.
// FUNCTION: GP4 0x005297ba
void __cdecl FUN_005297ba(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi)), 0xe);
    gp4::call_regs(0x00539944, c);

    const uint32_t args[2] = {0x2du, c.eax};
    gp4::Regs d = c;
    gp4::call_regs_stdcall(0x00503230, d, args);
    r->eflags = gp4::flags::merge(r->eflags, d.eflags);
}
GP4_IMPL(0x005297ba, FUN_005297ba, "regs(esi:ptr[0x40]:layout=rec_005297ba) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
