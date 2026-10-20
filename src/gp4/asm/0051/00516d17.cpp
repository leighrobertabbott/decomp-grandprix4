#include <gp4/gp4.h>
#include <gp4/flags.h>

// Index search: finds the first position in the byte table at ebx, among the first [0x0066d8ec]
// entries, whose byte equals al. Found: eax = the index and CF = 0 (the other flags are those of the
// equal `cmp al, [table + index]`); not found: eax = 0 and CF = 1 (the other flags those of the final
// `cmp index, count`, equal). edi is pushed and restored. The count is never 0 (the loop would
// run 4G entries).
// FUNCTION: GP4 0x00516d17
void __cdecl FUN_00516d17(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint8_t* table = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t index = 0u;
    uint32_t arith;
    for (;;) {
        const uint8_t entry = table[index];
        arith = gp4::flags::sub8(key, entry);                          // cmp al, [edi + ebx]
        if (key == entry) {
            r->eax = index;
            arith &= ~gp4::flags::CF;                                  // clc
            break;
        }
        index += 1u;
        arith = gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x0066d8ec));   // cmp edi, [count]
        if (index == GP4_GLOBAL(uint32_t, 0x0066d8ec)) {
            r->eax = 0u;
            arith |= gp4::flags::CF;                                   // stc
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516d17, FUN_00516d17, "regs(eax:u32[0..30], ebx:u32{=0x675958}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=order_00675958")
