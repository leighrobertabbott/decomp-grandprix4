#include <gp4/gp4.h>
#include <gp4/flags.h>

// Node of the singly linked list reached through 0x84300a; only the next pointer and the flag
// byte at +0x1f are used here.
struct Unknown_0052c55a_Node {
    uint32_t Next;
    uint8_t Unknown04[0x1b];
    uint8_t Flags1F;
};

// FUNCTION: GP4 0x0052c55a
// Walks the list from [0x84300a] for ten links, then for the next four nodes sets bit 0x20 of the
// node's byte +0x1f when the matching byte of the 0x34-stride array at 0x7ab684 equals 4, clears
// it otherwise. pushal/popal keep every register; the flags are those of the final `dec ecx`
// (1 -> 0) with CF still clear from `add ebx, 0x34`: ZF = PF = 1.
void __cdecl FUN_0052c55a(gp4::Regs* r) {
    uint32_t link = 0x0084300au;
    for (int i = 0; i < 10; ++i) {
        link = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(link));
    }
    const uint8_t* state = GP4_ARRAY(uint8_t, 0x007ab684);
    for (int k = 0; k < 4; ++k) {
        Unknown_0052c55a_Node* node = reinterpret_cast<Unknown_0052c55a_Node*>(static_cast<uintptr_t>(link));
        if (*state == 4u) {
            node->Flags1F = static_cast<uint8_t>(node->Flags1F | 0x20u);
        } else {
            node->Flags1F = static_cast<uint8_t>(node->Flags1F & 0xdfu);
        }
        link = node->Next;
        state += 0x34;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x0052c55a, FUN_0052c55a, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+flags_0052c55a")
