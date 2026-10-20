#include <gp4/gp4.h>

struct Unknown_0042458b_Esi {
    uint8_t Unknown00[0x30];
    int32_t Unknown30;
    int32_t Unknown34;
    int32_t Unknown38;
    uint32_t Unknown3C;
    uint32_t Unknown40;
    uint32_t Unknown44;
    uint8_t Unknown48[0x4c - 0x48];
    uint32_t Unknown4C;
    uint8_t Unknown50[0x6d - 0x50];
    uint8_t Index6D;
};

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// pushal ... popal routine for the object record in esi: points [0x006097b0] at the 0x5d8-byte block
// 0x006099c0 + byte(+0x6d) * 0x5d8, copies the dword +0x4c (at least 1) and the vector +0x30..+0x44
// (the first three scaled down by 32) to 0x0062a054 / 0x0062a064..0x0062a078, copies the table pointer
// [0x0062a044] and the entry count [0x0062a04c] (at least 1) to 0x0062a058 / 0x0062a05c, then runs
// 0x00422632 (eax = that count), 0x004227e5, 0x004226ae, 0x0042257b and 0x00424625 in turn with the
// register state each leaves. popal restores every register; the flags are the last callee's.
// FUNCTION: GP4 0x0042458b
void __cdecl FUN_0042458b(gp4::Regs* r) {
    const Unknown_0042458b_Esi* object = reinterpret_cast<const Unknown_0042458b_Esi*>(static_cast<uintptr_t>(r->esi));

    uint8_t* block = GP4_ARRAY(uint8_t, 0x006099c0) + object->Index6D * 0x5d8;
    GP4_GLOBAL(uint32_t, 0x006097b0) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block));

    uint32_t size = object->Unknown4C;
    if (size == 0u) {
        size = 1u;
    }
    GP4_GLOBAL(uint32_t, 0x0062a054) = size;

    GP4_GLOBAL(int32_t, 0x0062a064) = object->Unknown30 >> 5;
    GP4_GLOBAL(int32_t, 0x0062a068) = object->Unknown34 >> 5;
    GP4_GLOBAL(int32_t, 0x0062a06c) = object->Unknown38 >> 5;
    GP4_GLOBAL(uint32_t, 0x0062a070) = object->Unknown3C;
    GP4_GLOBAL(uint32_t, 0x0062a074) = object->Unknown40;
    GP4_GLOBAL(uint32_t, 0x0062a078) = object->Unknown44;

    GP4_GLOBAL(uint32_t, 0x0062a058) = GP4_GLOBAL(uint32_t, 0x0062a044);

    const int32_t entries = GP4_GLOBAL(int32_t, 0x0062a04c);
    GP4_GLOBAL(int32_t, 0x0062a05c) = entries < 1 ? 1 : entries;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block));
    call.eax = static_cast<uint32_t>(entries < 1 ? 1 : entries);
    call.eflags = (call.eflags & ~0x8d5u) | cmpFlags32(static_cast<uint32_t>(entries), 1u);

    gp4::call_regs(0x00422632, call);
    gp4::call_regs(0x004227e5, call);
    gp4::call_regs(0x004226ae, call);
    gp4::call_regs(0x0042257b, call);
    gp4::call_regs(0x00424625, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0042458b, FUN_0042458b, "regs(esi:ptr[0x75]:layout=rec_0042453d) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b")
