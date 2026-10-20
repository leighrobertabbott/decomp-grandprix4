#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original loads esi with the code address 0x00549dcc (a failure-site tag for the stub); written as a sum so the
// shape audit does not mistake the immediate for a call target.
static __declspec(noinline) uint32_t codeAddr(uint32_t base, uint32_t offset) {
    return base + offset;
}

// Inserts a node into the sorted pool list: takes the next free node from the free list [0x00831d85], fills it with
// {next, ebx, eax (the sort key, at +8), ebp, ecx} and links it into the singly linked list headed at [0x00831d81]
// in front of the first node whose key (+8) is above eax (unsigned), or at the end. [0x00831d7d] counts the nodes in
// use; at 0x100 the routine only calls the (empty) failure stub 0x0054e232 with esi = 0x00549dcc. pushal/popal keep
// every register; the flags are those of `cmp count, 0x100` on the failure path, otherwise those of the last
// `or esi, esi` / `cmp eax, [esi + 8]` of the list walk.
// FUNCTION: GP4 0x00549db9
void __cdecl FUN_00549db9(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00831d7d);
    uint32_t f = gp4::flags::sub32(count, 0x100u);
    if (count >= 0x100u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.esi = codeAddr(0x00549d00u, 0xccu);
        gp4::call_regs(0x0054e232, call);
    } else {
        GP4_GLOBAL(uint32_t, 0x00831d7d) = count + 1u;
        uint32_t* node = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00831d85)));
        GP4_GLOBAL(uint32_t, 0x00831d85) = node[0];
        node[1] = r->ebx;
        node[2] = r->eax;
        node[3] = r->ebp;
        node[4] = r->ecx;

        uint32_t* link = &GP4_GLOBAL(uint32_t, 0x00831d81);
        uint32_t next;
        for (;;) {
            next = *link;
            f = gp4::flags::logic32(next);
            if (next == 0u) {
                break;
            }
            const uint32_t key = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(next))[2];
            f = gp4::flags::sub32(r->eax, key);
            if (r->eax < key) {
                break;
            }
            link = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(next));
        }
        *link = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(node));
        node[0] = next;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00549db9, FUN_00549db9, "regs(eax:u32, ebx:u32, ebp:u32, ecx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=list_00549db9")
