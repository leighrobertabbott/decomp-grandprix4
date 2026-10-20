#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Files one 10-byte record (dword 0x00962f9c, dword 0x00962fa0, word 0x00962fa4) at slot
// [0x009559f4] of the 0x200-slot table at 0x00961b9c and bumps the slot counter; a full table
// (counter + 1 >= 0x200 as an unsigned 32-bit sum, so 0xffffffff wraps to 0 and passes) executes
// int3. eax returns the slot used (the old counter); edx is the staging dword 0x00962fa0 with its
// low word replaced by the word 0x00962fa4 (`mov dx, ...`); the flags are those of `mul edx`
// (index * 10): CF = OF = high half nonzero, AF = 0, ZF/SF/PF of the low half.
// FUNCTION: GP4 0x00559024
void __cdecl FUN_00559024(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x009559f4);
    if (index + 1u >= 0x200u) {
        __debugbreak();
        return;
    }
    GP4_GLOBAL(uint32_t, 0x009559f4) = index + 1u;
    const uint64_t product = static_cast<uint64_t>(index) * 10u;
    const uint32_t offset = static_cast<uint32_t>(product);
    uint8_t* const slot = GP4_ARRAY(uint8_t, 0x00961b9c) + offset;

    GP4_FIELD(uint32_t, slot, 0) = GP4_GLOBAL(uint32_t, 0x00962f9c);
    const uint32_t middle = GP4_GLOBAL(uint32_t, 0x00962fa0);
    GP4_FIELD(uint32_t, slot, 4) = middle;
    const uint16_t tail = GP4_GLOBAL(uint16_t, 0x00962fa4);
    GP4_FIELD(uint16_t, slot, 8) = tail;

    uint32_t flags = gp4::flags::logic32(offset);
    if ((product >> 32) != 0u) {
        flags |= 0x801u;  // CF and OF
    }
    r->eax = index;
    r->edx = (middle & 0xffff0000u) | tail;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00559024, FUN_00559024, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_009559f4")
