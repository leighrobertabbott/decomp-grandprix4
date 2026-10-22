#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that draws an edit record. esi points at a record whose words +4/+6/+8/+0xa are
// the text box corners (stored to 0x00838c18 / 0x00838c1c / 0x00838c20 / 0x00838c24) and whose dword
// +0xe is the text-edit record handed to the text drawer 0x0054e093 (also kept in 0x00838c14). The
// drawing colour is first set to 0x7fff through 0x0054cc96; the corners (ebx, eax, ebp, ecx) are
// passed to the sorted-list insert 0x00549db9 after the empty routine 0x00531b3d. popal restores
// every register, so only the flags remain: those of the last flag writer inside 0x0054e093.
// The record at esi is exercised with its +0xe dword pointing into the text-script pointer table at 0x00835846 (a
// {start, ?, cursor} triple of valid script pointers). ebx/ecx/edx reach the colour setter 0x0054cc96 untouched.
// FUNCTION: GP4 0x0054dc36
void __cdecl FUN_0054dc36(gp4::Regs* r) {
    const uint32_t record = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);

    c.ebx = GP4_FIELD(uint16_t, record, 4);
    c.eax = GP4_FIELD(uint16_t, record, 6);
    c.ebp = GP4_FIELD(uint16_t, record, 8);
    c.ecx = GP4_FIELD(uint16_t, record, 0xa);
    GP4_GLOBAL(uint32_t, 0x00838c18) = c.ebx;
    GP4_GLOBAL(uint32_t, 0x00838c1c) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00838c20) = c.ebp;
    GP4_GLOBAL(uint32_t, 0x00838c24) = c.ecx;
    gp4::call_regs(0x00531b3d, c);
    gp4::call_regs(0x00549db9, c);

    c.edi = GP4_FIELD(uint32_t, record, 0xe);
    GP4_GLOBAL(uint32_t, 0x00838c14) = c.edi;
    gp4::call_regs(0x0054e093, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054dc36, FUN_0054dc36, "regs(ebx:u32, ecx:u32, edx:u32, esi:ptr[0x20]:u32{=0x58460083,0x584a0083,0x584e0083,0x58520083,0x58560083,0x585a0083,0x585e0083,0x58620083}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+script_all_00835846+textout_0054d211+log_004fd730")
