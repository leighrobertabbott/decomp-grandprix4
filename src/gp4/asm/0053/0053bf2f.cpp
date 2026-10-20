#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-item draw step: runs the text interpreter 0x0054d798 on the string id in the dword [esi + 0x12]
// (output buffer edi = 0x00823100), reads the item's value cell through 0x00539944 (width by the byte
// [esi + 0xd] & 3, from [ebx]), runs the interpreter again on the string id value + 0x491, and finishes
// with the 0x0053c135 blit. esi is saved and restored around each call; registers and flags thread
// through every callee, the flags of the last callee are the result.
// FUNCTION: GP4 0x0053bf2f
void __cdecl FUN_0053bf2f(gp4::Regs* r) {
    const uint32_t item = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823100u;
    c.esi = GP4_FIELD(uint32_t, item, 0x12);
    gp4::call_regs(0x0054d798, c);

    c.esi = item;
    gp4::call_regs(0x00539944, c);

    const uint32_t arith = gp4::flags::add32(c.eax, 0x491u);
    c.eax += 0x491u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    c.esi = c.eax;
    gp4::call_regs(0x0054d798, c);

    c.esi = item;
    gp4::call_regs(0x0053c135, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0053bf2f, FUN_0053bf2f, "regs(eax:u32, ebx:ptr[8]:layout=cell_0053bf2f, ecx:u32, edx:u32, esi:ptr[0x40]:layout=item_0053bf2f, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+strs_0053bf2f+list_00549db9")
