#include <gp4/gp4.h>
#include <gp4/flags.h>

// One of the 0x400 six-dword slots at 0x829cac (fields named only by offset: +0x08 is the start of
// the slot's block in the pool, +0x0c its length and in-use word).
struct Unknown_00538921_Slot {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
};

static __declspec(noinline) uint8_t* advance_byte(uint8_t* p) {
    return p + 1;
}

// FUNCTION: GP4 0x00538921
// Removes the block of slot eax from the pool ([0x82fcac] is the used end): the slot's length (+0x0c)
// is cleared, the bytes after the block (start + length .. [0x82fcac]) are moved down over it with a
// forward `rep movsb`, every in-use slot (+0x0c != 0) whose start (+0x08) is not below the removed
// block's start moves down by the length, and [0x82fcac] shrinks by the length. All GPRs are saved
// and restored; the flags are those of the final `sub [0x82fcac], ebx`.
void __cdecl FUN_00538921(gp4::Regs* r) {
    Unknown_00538921_Slot* const target = reinterpret_cast<Unknown_00538921_Slot*>(static_cast<uintptr_t>(r->eax));
    const uint32_t length = target->Unknown0C;
    const uint32_t start = target->Unknown08;
    target->Unknown0C = 0u;

    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start));
    uint8_t* src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start + length));
    uint32_t count = GP4_GLOBAL(uint32_t, 0x0082fcac) - (start + length);
    while (count != 0u) {
        *dst = *src;
        dst = advance_byte(dst);
        src = advance_byte(src);
        --count;
    }

    Unknown_00538921_Slot* slot = GP4_ARRAY(Unknown_00538921_Slot, 0x00829cac);
    for (int i = 0x400; i != 0; --i) {
        if (slot->Unknown0C != 0u && slot->Unknown08 >= start) {
            slot->Unknown08 = slot->Unknown08 - length;
        }
        ++slot;
    }

    const uint32_t end = GP4_GLOBAL(uint32_t, 0x0082fcac);
    GP4_GLOBAL(uint32_t, 0x0082fcac) = end - length;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(end, length));
}
GP4_IMPL(0x00538921, FUN_00538921, "regs(eax:rec[0x829cac,0x18,0..1023]) -> (cf, pf, af, zf, sf, of); globals=layout=pool_00538921")
