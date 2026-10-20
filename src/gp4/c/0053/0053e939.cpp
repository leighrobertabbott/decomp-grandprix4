#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the singly linked list that starts at esi (dword +0 = next, 0 ends the list; the head node itself is
// not examined). Every node reached with bit 0x40 set in its flag byte +0x1e is recorded in the 0x20-byte
// entry table at 0x822300: +0 node, +4 ordinal (1-based hop count), +8/+0xc the words at node +4/+6,
// +0x10/+0x14 the words at node +8/+0xa, +0x18/+0x1c their averages ((+4 + +8) >> 1, (+6 + +0xa) >> 1).
// The number of entries goes to [0x82fefb]. On exit esi is the last node, ebx the hop count, ecx the entry
// count, edi the end of the table, eax/edx the last values loaded; the flags are those of `cmp dword [esi], 0`.
// FUNCTION: GP4 0x0053e939
void __cdecl FUN_0053e939(gp4::Regs* r) {
    uint32_t node = r->esi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t* entry = GP4_ARRAY(uint32_t, 0x00822300);
    uint32_t count = 0;
    uint32_t hops = 0;

    while (GP4_FIELD(uint32_t, node, 0) != 0u) {
        node = GP4_FIELD(uint32_t, node, 0);
        hops++;
        if ((GP4_FIELD(uint8_t, node, 0x1e) & 0x40u) != 0u) {
            count++;
            entry[0] = node;
            entry[1] = hops;
            eax = GP4_FIELD(uint16_t, node, 4);
            entry[2] = eax;
            edx = GP4_FIELD(uint16_t, node, 8);
            entry[4] = edx;
            eax = (eax + edx) >> 1;
            entry[6] = eax;
            eax = GP4_FIELD(uint16_t, node, 6);
            entry[3] = eax;
            edx = GP4_FIELD(uint16_t, node, 0xa);
            entry[5] = edx;
            eax = (eax + edx) >> 1;
            entry[7] = eax;
            entry += 8;
        }
    }
    GP4_GLOBAL(uint32_t, 0x0082fefb) = count;

    r->eax = eax;
    r->ebx = hops;
    r->ecx = count;
    r->edx = edx;
    r->esi = node;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0053e939, FUN_0053e939, "regs(eax:u32, edx:u32, esi:ptr[0x28]:layout=lnode1_0053e612) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of)")
