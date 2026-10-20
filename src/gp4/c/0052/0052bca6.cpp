#include <gp4/gp4.h>
#include <gp4/flags.h>

// A linked-list node: the next pointer first, a flag byte at +0x1f.
struct Unknown_ListNode {
    Unknown_ListNode* Next;
    uint8_t Unknown04[0x1b];
    uint8_t Flags1F;
};

// FUNCTION: GP4 0x0052bca6
// Walks the list that starts at 0x0084d8ec 0x31 links in, then updates bit 0x20 of the flag byte
// +0x1f of the next five nodes: all five get the bit set when [0x007a8e0d] or [0x0081d615] is
// zero. Otherwise the first node's bit is cleared and the four after it are set (bit 0 of
// [0x0081d616] clear) or cleared (bit 0 set). Ends with esi = the node after the fifth and
// ecx = 0; the flags are those of the last `dec ecx` (CF clear from the preceding or/and).
void __cdecl FUN_0052bca6(gp4::Regs* r) {
    Unknown_ListNode* node = reinterpret_cast<Unknown_ListNode*>(GP4_ARRAY(uint8_t, 0x0084d8ec));
    for (uint32_t i = 0x31; i != 0u; --i) {
        node = node->Next;
    }

    if (GP4_GLOBAL(uint8_t, 0x007a8e0d) == 0u || GP4_GLOBAL(uint8_t, 0x0081d615) == 0u) {
        for (uint32_t i = 5; i != 0u; --i) {
            node->Flags1F |= 0x20u;
            node = node->Next;
        }
    } else {
        node->Flags1F &= 0xdfu;
        node = node->Next;
        if ((GP4_GLOBAL(uint8_t, 0x0081d616) & 1u) == 0u) {
            for (uint32_t i = 4; i != 0u; --i) {
                node->Flags1F |= 0x20u;
                node = node->Next;
            }
        } else {
            for (uint32_t i = 4; i != 0u; --i) {
                node->Flags1F &= 0xdfu;
                node = node->Next;
            }
        }
    }

    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(node));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x0052bca6, FUN_0052bca6, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); globals=layout=list_0084d8ec+flags_0052bca6")
