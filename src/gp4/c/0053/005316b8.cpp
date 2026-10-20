#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scales a value down through the registered callback table. eax starts as the value looked up by
// 0x00531859; ecx counts the entries visited. While eax > 0x1249 and ecx < 7 the second routine of
// the current 12-byte entry (offset +4; table 0x8274b1 when byte [0x7ad869] is set, else
// 0x827505) is called; when it returns with CF clear eax = eax * entry.scale / 100 (unsigned mul
// then div, remainder left in edx). The final count is stored to [0x8274a9]. The flags are those
// of the last compare (`cmp eax, 0x1249` or `cmp ecx, 7`).
// FUNCTION: GP4 0x005316b8
void __cdecl FUN_005316b8(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00531859, c);

    uint32_t count = 0u;
    c.esi = GP4_GLOBAL(uint8_t, 0x007ad869) ? 0x008274b1u : 0x00827505u;
    uint32_t flags;
    for (;;) {
        flags = gp4::flags::sub32(c.eax, 0x1249u);
        if (c.eax <= 0x1249u) {
            break;
        }
        flags = gp4::flags::sub32(count, 7u);
        if (count >= 7u) {
            break;
        }
        c.ecx = count;
        gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi + 4u)), c);
        if (!(c.eflags & gp4::CF)) {
            const uint32_t scale = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi + 8u));
            uint32_t remainder = 0u;
            c.eax = gp4::div64((uint64_t)c.eax * (uint64_t)scale, 100u, &remainder);
            c.edx = remainder;
        }
        c.esi += 0xcu;
        ++count;
    }
    c.ecx = count;
    GP4_GLOBAL(uint32_t, 0x008274a9) = count;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x005316b8, FUN_005316b8, "regs(ebx:ptr[0x40]:bytes) -> (eax, ecx, edx, esi, ebx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=cb_00531648+sel_00531859")
