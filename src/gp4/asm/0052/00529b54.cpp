#include <gp4/gp4.h>
#include <gp4/flags.h>

// Table-driven byte poke: ebx points at triplets {hops, offset, value} ended by a zero hops byte.
// For each triplet the pointer chain starting at esi is followed `hops` times ([node] is the next
// node), then `value` is stored at node + offset. The whole routine is inside pushal/popal, so the
// registers come back unchanged; only the flags survive. The last flag writer is the `inc ebx` that
// reads the terminating zero byte (inc keeps CF, which is the carry of the last `add esi, eax`, or the
// incoming CF when the list is empty).
// FUNCTION: GP4 0x00529b54
void __cdecl FUN_00529b54(gp4::Regs* r) {
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t root = r->esi;
    uint32_t carry = r->eflags & static_cast<uint32_t>(gp4::flags::CF);

    for (;;) {
        uint32_t hops = cursor[0];
        if (hops == 0u) {
            break;
        }
        uint32_t node = root;
        do {
            node = GP4_GLOBAL(uint32_t, node);
            --hops;
        } while (hops != 0u);
        const uint32_t offset = cursor[1];
        const uint8_t value = cursor[2];
        cursor += 3;
        const uint32_t target = node + offset;
        carry = (target < node) ? static_cast<uint32_t>(gp4::flags::CF) : 0u;
        GP4_GLOBAL(uint8_t, target) = value;
    }

    r->eflags = gp4::flags::merge(
        r->eflags, gp4::flags::inc32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor)), carry));
}
GP4_IMPL(0x00529b54, FUN_00529b54, "regs(ebx:ptr[0x3c]:u8{=0x10,0x20,0x30}, esi:ptr[4669]:layout=chain_0084d8ec, cf:u32[0..1]) -> (cf, pf, af, zf, sf, of)")
