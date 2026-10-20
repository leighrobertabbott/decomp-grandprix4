#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the registered callback table (12-byte entries {fn0, fn1, scale}; 0x8274b1 when byte
// [0x7ad869] is set, else 0x827505; [0x8274ad] entries). For every entry it calls fn1 (offset +4);
// when that returns with CF clear, eax = eax * scale / 100 (unsigned mul then div by 100, the
// remainder stays in edx). ecx, edx and esi are saved and restored, so only eax, the registers the
// callbacks write and the flags (those of the final "or ecx,ecx" with ecx = 0) change.
// FUNCTION: GP4 0x00531679
void __cdecl FUN_00531679(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x008274ad);
    uint32_t entry = GP4_GLOBAL(uint8_t, 0x007ad869) ? 0x008274b1u : 0x00827505u;
    gp4::Regs c = *r;
    const uint32_t savedEcx = r->ecx;
    const uint32_t savedEdx = r->edx;
    const uint32_t savedEsi = r->esi;
    while (count != 0u) {
        c.ecx = count;
        c.esi = entry;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(count));
        gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry + 4u)), c);
        if (!(c.eflags & gp4::CF)) {
            const uint32_t scale = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi + 8u));
            uint32_t remainder = 0u;
            c.eax = gp4::div64((uint64_t)c.eax * (uint64_t)scale, 0x64u, &remainder);
            c.edx = remainder;
        }
        entry = c.esi + 0xcu;
        count = count - 1u;
    }
    *r = c;
    r->ecx = savedEcx;
    r->edx = savedEdx;
    r->esi = savedEsi;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00531679, FUN_00531679, "regs(eax:u32[0..0xffff], ebx:ptr[0x40]:bytes) -> (eax, ebx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=cb_00531648")
