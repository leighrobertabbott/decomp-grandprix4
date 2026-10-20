#include <gp4/gp4.h>

// For each of the DAT_00609864 records (0x64 bytes) at 0x00621aa0: stores its address in
// DAT_00609860 and runs the four per-record steps 0x004020d8, 0x00425321, 0x004253e3 and
// 0x00425395 in turn. The registers the steps change carry from one to the next (and into the next
// record); ecx is the loop counter, saved around the calls. The flags are ZF|PF in every case: the
// test of a zero count, or the final equal "cmp ecx, [0x609864]".
// FUNCTION: GP4 0x00425431
void __cdecl FUN_00425431(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00609864);
    if (count != 0u) {
        gp4::Regs state = *r;
        state.st_in = 0;
        state.st_out = 0;

        uint32_t index = 0;
        do {
            const uint32_t record = index * 0x64u + 0x00621aa0u;
            state.ecx = index;
            state.edi = record;
            GP4_GLOBAL(uint32_t, 0x00609860) = record;
            gp4::call_regs(0x004020d8, state);
            gp4::call_regs(0x00425321, state);
            gp4::call_regs(0x004253e3, state);
            gp4::call_regs(0x00425395, state);
            ++index;
        } while (index != GP4_GLOBAL(uint32_t, 0x00609864));

        r->eax = state.eax;
        r->ebx = state.ebx;
        r->ecx = index;
        r->edx = state.edx;
        r->edi = state.edi;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00425431, FUN_00425431, "regs() -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=view_00609860+recs_00621aa0")
