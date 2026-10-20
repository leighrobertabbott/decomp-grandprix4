// Native check of include/gp4/flags.h against the host CPU (defined flags only).
#include <gp4/flags.h>
#include <stdio.h>
#include <stdint.h>

static uint32_t rng_state = 0x12345678u;
static uint32_t rnd() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static uint32_t hw_sub32(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { cmp eax, b } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_add32(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { add eax, b } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_sub16(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { mov ecx, b } __asm { cmp ax, cx } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_add16(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { mov ecx, b } __asm { add ax, cx } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_sub8(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { mov ecx, b } __asm { cmp al, cl } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_add8(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { mov ecx, b } __asm { add al, cl } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_neg32(uint32_t a) { uint32_t f; __asm { mov eax, a } __asm { neg eax } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_and32(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { and eax, b } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_and8(uint32_t a, uint32_t b) { uint32_t f; __asm { mov eax, a } __asm { mov ecx, b } __asm { and al, cl } __asm { pushfd } __asm { pop f } return f; }
static uint32_t hw_inc32(uint32_t a, uint32_t cf) {
    uint32_t f;
    __asm { mov eax, cf } __asm { shr eax, 1 } __asm { mov eax, a } __asm { inc eax } __asm { pushfd } __asm { pop f }
    return f;
}
static uint32_t hw_dec32(uint32_t a, uint32_t cf) {
    uint32_t f;
    __asm { mov eax, cf } __asm { shr eax, 1 } __asm { mov eax, a } __asm { dec eax } __asm { pushfd } __asm { pop f }
    return f;
}

static int fails = 0;
static void check(const char* what, uint32_t a, uint32_t b, uint32_t hw, uint32_t model, uint32_t mask) {
    if ((hw & mask) != (model & mask)) {
        if (fails < 20) printf("FAIL %s a=%08x b=%08x hw=%03x model=%03x\n", what, a, b, hw & mask, model & mask);
        ++fails;
    }
}

int main() {
    using namespace gp4::flags;
    const uint32_t edge[] = {0u, 1u, 2u, 0x7fu, 0x80u, 0xffu, 0x7fffu, 0x8000u, 0xffffu, 0x7fffffffu,
                             0x80000000u, 0xfffffffeu, 0xffffffffu, 0x10u, 0xfu};
    const uint32_t logic_mask = CF | PF | ZF | SF | OF;   // AF undefined for and
    for (int i = 0; i < 200000; ++i) {
        uint32_t a = rnd(), b = rnd();
        if (i < 225) { a = edge[i % 15]; b = edge[i / 15]; }
        else if (i & 1) { a &= 0xffu; b &= 0x1ffu; }
        check("sub32", a, b, hw_sub32(a, b), sub32(a, b), ARITH);
        check("add32", a, b, hw_add32(a, b), add32(a, b), ARITH);
        check("sub16", a, b, hw_sub16(a, b), sub16(a, b), ARITH);
        check("add16", a, b, hw_add16(a, b), add16(a, b), ARITH);
        check("sub8", a, b, hw_sub8(a, b), sub8(a, b), ARITH);
        check("add8", a, b, hw_add8(a, b), add8(a, b), ARITH);
        check("neg32", a, 0, hw_neg32(a), neg32(a), ARITH);
        check("and32", a, b, hw_and32(a, b), logic32(a & b), logic_mask);
        check("and8", a, b, hw_and8(a, b), logic8(a & b), logic_mask);
        check("inc32", a, b & 1u, hw_inc32(a, b & 1u), inc32(a, b & 1u), ARITH);
        check("dec32", a, b & 1u, hw_dec32(a, b & 1u), dec32(a, b & 1u), ARITH);
    }
    if (fails) { printf("%d failures\n", fails); return 1; }
    printf("ALL PASS\n");
    return 0;
}
