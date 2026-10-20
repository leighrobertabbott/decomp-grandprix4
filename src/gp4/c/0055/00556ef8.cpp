#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads one pit-lane interpolation command from the word stream at esi. With the mode byte [0x0086857c]
// not 1 it just skips two words (esi + 4, flags of that add). Otherwise the first word dx is the step count:
// 0 starts a new segment (the next word becomes the start position [0x870604]/[0x870608], the end position
// [0x87060a] = start + 0x50, the count/step words are cleared; flags of the 16-bit `add ax, 0x50`); a
// nonzero count is kept at [0x870602]/[0x870606] and the next word (not consumed by the first read) gives
// two per-step deltas (word - [0x870604]) / count -> [0x87060c] and (word - [0x870608]) / count ->
// [0x87060e] (signed 16-bit differences, idiv by the zero-extended count; flags of the last `sub ax, ..`,
// idiv leaves them). Ends with esi + 4, eax = last quotient, edx = last remainder, ebp = count.
// FUNCTION: GP4 0x00556ef8
void __cdecl FUN_00556ef8(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 4u));
        r->esi += 4u;
        return;
    }
    const uint16_t* stream = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint16_t count = stream[0];
    const uint16_t word = stream[1];
    r->esi += 4u;
    if (count == 0u) {
        GP4_GLOBAL(uint16_t, 0x00870604) = word;
        GP4_GLOBAL(uint16_t, 0x00870608) = word;
        const uint16_t end = static_cast<uint16_t>(word + 0x50u);
        GP4_GLOBAL(uint16_t, 0x0087060a) = end;
        GP4_GLOBAL(uint16_t, 0x00870602) = 0u;
        GP4_GLOBAL(uint16_t, 0x0087060c) = 0u;
        GP4_GLOBAL(uint16_t, 0x00870606) = 0u;
        GP4_GLOBAL(uint16_t, 0x0087060e) = 0u;
        r->eax = (r->eax & 0xffff0000u) | end;
        r->edx = (r->edx & 0xffff0000u) | count;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add16(word, 0x50u));
        return;
    }
    GP4_GLOBAL(uint16_t, 0x00870602) = count;
    GP4_GLOBAL(uint16_t, 0x00870606) = count;
    const uint16_t base_a = GP4_GLOBAL(uint16_t, 0x00870604);
    const int32_t diff_a = static_cast<int16_t>(static_cast<uint16_t>(word - base_a));
    const int32_t divisor_a = GP4_GLOBAL(uint16_t, 0x00870602);
    GP4_GLOBAL(uint16_t, 0x0087060c) = static_cast<uint16_t>(diff_a / divisor_a);
    const uint16_t base_b = GP4_GLOBAL(uint16_t, 0x00870608);
    const int32_t diff_b = static_cast<int16_t>(static_cast<uint16_t>(word - base_b));
    const int32_t divisor_b = GP4_GLOBAL(uint16_t, 0x00870606);
    const int32_t quotient = diff_b / divisor_b;
    const int32_t remainder = diff_b % divisor_b;
    GP4_GLOBAL(uint16_t, 0x0087060e) = static_cast<uint16_t>(quotient);
    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(divisor_b);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(word, base_b));
}
GP4_IMPL(0x00556ef8, FUN_00556ef8, "regs(eax:u32, edx:u32, ebp:u32, esi:ptr[0x10]:u16{0,0,1,2,3,0x50,0x100,0x7fff,0x8000,0xffff}) -> (eax, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
