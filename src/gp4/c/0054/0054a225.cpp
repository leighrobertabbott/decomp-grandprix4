#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 16-bit entries from [0x822260] (source) into [0x822258] (destination) until the
// destination reaches [0x82225c]. Per entry the original calls two (currently empty) helpers,
// 0x531b02 with the entry in eax and 0x531b01 returning the result word in ax, and indexes the
// dword table at eax with ebx, ecx and edx. eax is saved/restored around each call; ebx, ecx, edx
// are overwritten from the table; esi/edi walk the arrays; the flags are those of the final cmp.
// FUNCTION: GP4 0x0054a225
void __cdecl FUN_0054a225(gp4::Regs* r) {
    const uint32_t table = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x00822260);
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00822258);
    uint32_t flags = r->eflags;
    do {
        gp4::Regs split = {};
        split.eax = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
        split.ebx = ebx;
        split.ecx = ecx;
        split.edx = edx;
        split.esi = esi;
        split.edi = edi;
        split.ebp = r->ebp;
        split.eflags = flags;
        gp4::call_regs(0x00531b02, split);
        ebx = split.ebx;
        ecx = split.ecx;
        edx = split.edx;
        esi = split.esi;
        edi = split.edi;
        flags = split.eflags;

        ebx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(table + ebx * 4u));
        ecx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(table + ecx * 4u));
        edx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(table + edx * 4u));

        gp4::Regs join = {};
        join.eax = table;
        join.ebx = ebx;
        join.ecx = ecx;
        join.edx = edx;
        join.esi = esi;
        join.edi = edi;
        join.ebp = r->ebp;
        join.eflags = flags;
        gp4::call_regs(0x00531b01, join);
        ebx = join.ebx;
        ecx = join.ecx;
        edx = join.edx;
        esi = join.esi;
        edi = join.edi;
        *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(edi)) = static_cast<uint16_t>(join.eax);

        esi += 2u;
        edi += 2u;
        flags = gp4::flags::merge(join.eflags, gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x0082225c)));
    } while (edi != GP4_GLOBAL(uint32_t, 0x0082225c));
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = flags;
}
GP4_IMPL(0x0054a225, FUN_0054a225, "regs(eax:ptr[0x400]:i32[0..255], ebx:u32[0..255], ecx:u32[0..255], edx:u32[0..255]) -> (ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=surf_00822258")
