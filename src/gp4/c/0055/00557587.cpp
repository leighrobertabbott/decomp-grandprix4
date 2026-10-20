#include <gp4/gp4.h>

// FUNCTION: GP4 0x00557587
// pushal/pushf ... popf/popal: registers and flags are restored, only the two history tables change.
// Walks the stack slots of the byte tables 0x00875b46 and 0x00875c4a downwards from the top index
// [0x871c80] to the bottom index [0x871c7c] (signed; nothing happens when the top is below the bottom).
// For each slot the table 0x00875bc8 / 0x00875ccc receives the previous distinct value of the matching
// source table (a run-length "what came before" pass: bh/ch keep the value before the latest change, bl/cl the
// latest). When the carry flag the caller entered with is set, the top slot [0x871c80] is stored once more
// with the final bh/ch values.
void __cdecl FUN_00557587(gp4::Regs* r) {
    const int32_t top = GP4_GLOBAL(int32_t, 0x00871c80);
    const int32_t bottom = GP4_GLOBAL(int32_t, 0x00871c7c);
    if (top < bottom) {
        return;
    }
    int32_t index = GP4_GLOBAL(int32_t, 0x00871c80);
    uint8_t latestA = GP4_ARRAY(uint8_t, 0x00875b46)[index];
    uint8_t beforeA = latestA;
    uint8_t latestB = GP4_ARRAY(uint8_t, 0x00875c4a)[index];
    uint8_t beforeB = latestB;
    for (;;) {
        GP4_ARRAY(uint8_t, 0x00875bc8)[index] = beforeA;
        GP4_ARRAY(uint8_t, 0x00875ccc)[index] = beforeB;
        index -= 1;
        if (index < bottom) {
            break;
        }
        const uint8_t a = GP4_ARRAY(uint8_t, 0x00875b46)[index];
        if (latestA != a) {
            beforeA = latestA;
            latestA = a;
        }
        const uint8_t b = GP4_ARRAY(uint8_t, 0x00875c4a)[index];
        if (latestB != b) {
            beforeB = latestB;
            latestB = b;
        }
    }
    if ((r->eflags & 1u) != 0u) {
        const int32_t topIndex = GP4_GLOBAL(int32_t, 0x00871c80);
        GP4_ARRAY(uint8_t, 0x00875bc8)[topIndex] = beforeA;
        GP4_ARRAY(uint8_t, 0x00875ccc)[topIndex] = beforeB;
    }
}
GP4_IMPL(0x00557587, FUN_00557587, "regs(cf:u32[0..1]) -> (); globals=layout=stack_0055753f+bound_00557587")
