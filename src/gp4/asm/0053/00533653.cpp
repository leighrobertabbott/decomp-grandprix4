#include <gp4/gp4.h>
#include <gp4/flags.h>

// Allocates edx bytes of global memory (GlobalAlloc(0, edx)) and registers the block in the first free slot of the
// 16-entry table at 0x00828020 (0x005336c6 returns the slot in edi, CF = 1 when the table is full): the slot gets
// {edx, eax} = {size, block} and the counter [0x0082801c] is incremented. When GlobalAlloc fails (eax = 0) control
// jumps to the handler address stored in [0x008334d7] with the registers as GlobalAlloc left them. edx is kept
// around the allocation call; ecx/flags are whatever the callee leaves.
// FUNCTION: GP4 0x00533653
void __cdecl FUN_00533653(gp4::Regs* r) {
    const uint32_t size = r->edx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = size;
    const uint32_t allocArgs[2] = {0u, size};
    gp4::call_regs_stdcall(0x005a48d0, c, allocArgs);
    c.edx = size;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));

    if (c.eax == 0u) {
        gp4::call_regs(GP4_GLOBAL(uint32_t, 0x008334d7), c);
    } else {
        gp4::call_regs(0x005336c6, c);
        if ((c.eflags & gp4::CF) == 0u) {
            GP4_FIELD(uint32_t, c.edi, 0) = c.edx;
            GP4_FIELD(uint32_t, c.edi, 4) = c.eax;
            const uint32_t count = GP4_GLOBAL(uint32_t, 0x0082801c);
            GP4_GLOBAL(uint32_t, 0x0082801c) = count + 1u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(count, c.eflags));
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00533653, FUN_00533653, "regs(edx:u32{0,1,16,0x100,0x1234}, ecx:u32, edi:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 GlobalAlloc")
