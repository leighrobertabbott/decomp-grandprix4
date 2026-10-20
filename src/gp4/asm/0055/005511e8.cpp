#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005511e8
// Refreshes the second span (A, B) of the stack slot s = [0x871c90] used with the segment record
// edi: clears the blend weight [0x874b04], takes A = byte 0x875c4a[s] (stored in [0x871c9c],
// returned in ebx) and, unless the record pointer 0x875940[A] already equals edi, takes
// B = byte 0x875ccc[s] (stored in [0x871ca0], ecx) and runs the interpolation 0x0055108e for edi
// with ebx = A, ecx = B. esi and edi are restored (push/pop); the other registers and the flags
// are those of the cmp (equal case) or of the interpolation (its flags replace the cmp's).
void __cdecl FUN_005511e8(gp4::Regs* r) {
    const uint32_t record = r->edi;

    GP4_GLOBAL(uint32_t, 0x00874b04) = 0u;
    const uint32_t slot = GP4_GLOBAL(uint32_t, 0x00871c90);
    const uint32_t spanA = GP4_ARRAY(uint8_t, 0x00875c4a)[slot];
    GP4_GLOBAL(uint32_t, 0x00871c9c) = spanA;
    const uint32_t recordA = GP4_ARRAY(uint32_t, 0x00875940)[spanA];

    r->ebx = spanA;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, recordA));
    if (record == recordA) {
        return;
    }

    const uint32_t spanB = GP4_ARRAY(uint8_t, 0x00875ccc)[slot];
    GP4_GLOBAL(uint32_t, 0x00871ca0) = spanB;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = slot;
    c.ecx = spanB;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));   // xor ecx, ecx
    gp4::call_regs(0x0055108e, c);

    // esi and edi are popped back to their entry values; everything else comes from the callee.
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005511e8, FUN_005511e8, "regs(edi:rec[0x8943a0,0x160,0..7]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227")
