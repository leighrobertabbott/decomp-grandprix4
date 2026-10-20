#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the singly linked list headed through [0x0082fd34] (first node = [[0x0082fd34]], next =
// dword +0). For every node whose byte +0xc differs from the mode byte [0x0082fd45] it calls the
// node's handler (dword +0x1a, with edx = 1, ebx = node dword +0xe, esi = node) between pushal/popal
// and notes whether the handler changed the node's flag byte +0x1f. A node gets bit 0x40 set in
// +0x1f when the handler changed it, when [0x0081e4fb] == 0xff, or when its byte +0x1e has bit 5 and
// its word +0x20 shares a bit with the word +0x20 of the record in esi on entry.
// On return cx = that word, dh = byte +0x1e of the entry record, dl = [0x0082fd45], ah = the flag
// byte of the last node whose handler ran, esi = 0 and the flags are those of `or esi,esi`.
// FUNCTION: GP4 0x0053dc57
void __cdecl FUN_0053dc57(gp4::Regs* r) {
    uint8_t* self = (uint8_t*)(uintptr_t)r->esi;
    const uint16_t mask = GP4_FIELD(uint16_t, self, 0x20);
    const uint8_t selfFlags = GP4_FIELD(uint8_t, self, 0x1e);
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0082fd45);
    uint32_t eax = r->eax;

    uint8_t* node = (uint8_t*)(uintptr_t)GP4_GLOBAL(uint32_t, 0x0082fd34);
    for (;;) {
        node = (uint8_t*)(uintptr_t)GP4_FIELD(uint32_t, node, 0);
        if (node == nullptr) {
            break;
        }
        bool mark = false;
        bool decided = false;
        if (mode != GP4_FIELD(uint8_t, node, 0x0c)) {
            const uint8_t before = GP4_FIELD(uint8_t, node, 0x1f);
            eax = (eax & 0xffff00ffu) | ((uint32_t)before << 8);
            gp4::Regs c = *r;
            c.eax = eax;
            c.ecx = (r->ecx & 0xffff0000u) | mask;
            c.edx = 1u;
            c.ebx = GP4_FIELD(uint32_t, node, 0x0e);
            c.esi = (uint32_t)(uintptr_t)node;
            gp4::call_regs(GP4_FIELD(uint32_t, node, 0x1a), c);
            if (before != GP4_FIELD(uint8_t, node, 0x1f)) {
                mark = true;
                decided = true;
            }
        }
        if (!decided) {
            if (GP4_GLOBAL(uint8_t, 0x0081e4fb) == 0xffu) {
                mark = true;
            } else if ((GP4_FIELD(uint8_t, node, 0x1e) & 0x20u) != 0u &&
                       (GP4_FIELD(uint16_t, node, 0x20) & mask) != 0u) {
                mark = true;
            }
        }
        if (mark) {
            GP4_FIELD(uint8_t, node, 0x1f) = (uint8_t)(GP4_FIELD(uint8_t, node, 0x1f) | 0x40u);
        }
    }

    r->eax = eax;
    r->ecx = (r->ecx & 0xffff0000u) | mask;
    r->edx = (r->edx & 0xffff0000u) | ((uint32_t)selfFlags << 8) | mode;
    r->esi = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0053dc57, FUN_0053dc57, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+items_0053dc57")
