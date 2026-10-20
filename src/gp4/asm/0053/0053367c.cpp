#include <gp4/gp4.h>
#include <gp4/flags.h>

// Releases the global-memory block registered under the key in edx: the 16-entry table at 0x00828020
// is searched by 0x005336ef (key = dword +4 of an eight-byte entry); a hit clears the entry's first
// dword and decrements the counter 0x0082801c. Then GlobalHandle(key) (thunk 0x005a4924) and, when it
// returns a handle, GlobalFree(handle) (thunk 0x005a48d6) run with the live registers. eax/ecx/edx/
// flags are whatever the last call leaves; the `or eax, eax` flags are kept when GlobalHandle returns 0.
// FUNCTION: GP4 0x0053367c
void __cdecl FUN_0053367c(gp4::Regs* r) {
    const uint32_t key = r->edx;
    gp4::Regs c = *r;
    c.eax = key;
    gp4::call_regs(0x005336ef, c);

    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_FIELD(uint32_t, c.edi, 0) = 0u;
        const uint32_t old = GP4_GLOBAL(uint32_t, 0x0082801c);
        GP4_GLOBAL(uint32_t, 0x0082801c) = old - 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(old, c.eflags));
    }

    c.eax = key;
    const uint32_t handle_args[1] = {key};
    gp4::call_regs_stdcall(0x005a4924, c, handle_args);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        const uint32_t free_args[1] = {c.eax};
        gp4::call_regs_stdcall(0x005a48d6, c, free_args);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0053367c, FUN_0053367c, "regs(edx:u32{0,0,1,2,0x1234}, ecx:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
