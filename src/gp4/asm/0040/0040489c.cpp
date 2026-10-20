#include <gp4/gp4.h>

struct Unknown_0040489c {
    uint8_t Unknown00[0x98];
    uint32_t Length98;
    uint8_t Unknown9C[0xa4 - 0x9c];
    int32_t ComponentA4;
    int32_t ComponentA8;
};

// Length of the vector (+0xa4, +0xa8): the sum of the two 64-bit squares goes through the integer
// square root 0x0040113f and the result is stored at +0x98. The first square's high half is left
// in ebp, the sum's high half in edx; ebx and ecx are saved and restored.
// FUNCTION: GP4 0x0040489c
void __cdecl FUN_0040489c(gp4::Regs* r) {
    Unknown_0040489c* body = reinterpret_cast<Unknown_0040489c*>(static_cast<uintptr_t>(r->esi));

    const int64_t squareA = static_cast<int64_t>(body->ComponentA4) * static_cast<int64_t>(body->ComponentA4);
    const uint32_t lowA = static_cast<uint32_t>(squareA);
    const uint32_t highA = static_cast<uint32_t>(static_cast<uint64_t>(squareA) >> 32);

    const int64_t squareB = static_cast<int64_t>(body->ComponentA8) * static_cast<int64_t>(body->ComponentA8);
    const uint32_t lowB = static_cast<uint32_t>(squareB);
    const uint32_t highB = static_cast<uint32_t>(static_cast<uint64_t>(squareB) >> 32);

    const uint32_t low = lowB + lowA;
    const uint32_t carry = (low < lowB) ? 1u : 0u;
    const uint32_t high = highB + highA + carry;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = low;
    callee.edx = high;
    gp4::call_regs(0x0040113f, callee);

    body->Length98 = callee.eax;

    r->eax = callee.eax;
    r->edx = high;
    r->ebp = highA;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040489c, FUN_0040489c, "regs(esi:ptr[0xb0]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")
