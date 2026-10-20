#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the pair routine 0x00555941 (edi = record, ebx = the following record) over the 0x160-byte track-segment ring
// at 0x008943a0: edi walks 0x008943a0 up to the lap end [0x00866ecc] with ebx one record ahead (starting at 0x00894500),
// then edi walks the pit lane from [0x00891500] to the pit end [0x00866ef8] with ebx = edi + 0x160 at the start. Both
// walks are do-while loops that step both pointers by 0x160 and stop when edi equals the end. pushal/popal keep every
// register; the flags are those of the last `cmp edi, [0x00866ef8]` (equal operands).
// FUNCTION: GP4 0x00555b28
void __cdecl FUN_00555b28(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    uint32_t edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x008943a0)));
    uint32_t ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00894500)));
    do {
        call.edi = edi;
        call.ebx = ebx;
        gp4::call_regs(0x00555941, call);
        ebx += 0x160u;
        edi += 0x160u;
    } while (edi != GP4_GLOBAL(uint32_t, 0x00866ecc));

    edi = GP4_GLOBAL(uint32_t, 0x00891500);
    ebx = edi + 0x160u;
    uint32_t f;
    do {
        call.edi = edi;
        call.ebx = ebx;
        gp4::call_regs(0x00555941, call);
        ebx += 0x160u;
        edi += 0x160u;
        f = gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x00866ef8));
    } while (edi != GP4_GLOBAL(uint32_t, 0x00866ef8));

    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00555b28, FUN_00555b28, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_00555b28")
