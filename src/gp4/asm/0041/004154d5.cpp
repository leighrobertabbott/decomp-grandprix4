#include <gp4/gp4.h>

// edi: a pair of words, the second (+2) is the input scale.
struct Unknown_004154d5_Input {
    int16_t Unknown00;
    int16_t Unknown02;
};

// esi: a car record (only the fields read here are named).
struct Unknown_004154d5_Car {
    uint8_t Unknown00[0xe5];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x230 - 0xe6];
    uint8_t Flags230;
    uint8_t Unknown231[0x288 - 0x231];
    int32_t Unknown288;
};

// FUNCTION: GP4 0x004154d5
// [0x603d48] = ((word edi+2 * [0x622481]) >> 14) * [0x603854] >> 2; [0x62fbf0] = 0x004284e1(eax = [0x624848]);
// [0x62fbf4] = (esi dword +0x288 * [0x603854]) >> 18; then the weight ((the 0x004284e1 result, or
// [0x62fbf4] when [0x624848] is zero) * [0x603ac0]) >> 14 is made positive into ecx, scaled by the word
// table entry [0x7ad89e] of 0x602b30 when the car has +0xe5 bit 2 and +0x230 bit 6, and handed to
// 0x00427880. The registers are what 0x00427880 leaves (the other ones what 0x004284e1 left).
void __cdecl FUN_004154d5(gp4::Regs* r) {
    const Unknown_004154d5_Input* input = reinterpret_cast<const Unknown_004154d5_Input*>(static_cast<uintptr_t>(r->edi));

    int64_t product = static_cast<int64_t>(static_cast<int32_t>(input->Unknown02)) * GP4_GLOBAL(int32_t, 0x00622481);
    const int32_t scaled = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);
    product = static_cast<int64_t>(scaled) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    GP4_GLOBAL(uint32_t, 0x00603d48) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 2);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00624848);
    c.edx = high;
    gp4::call_regs(0x004284e1, c);
    const uint32_t result = c.eax;
    GP4_GLOBAL(uint32_t, 0x0062fbf0) = result;

    const Unknown_004154d5_Car* car = reinterpret_cast<const Unknown_004154d5_Car*>(static_cast<uintptr_t>(c.esi));
    product = static_cast<int64_t>(car->Unknown288) * GP4_GLOBAL(int32_t, 0x00603854);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 18);
    GP4_GLOBAL(uint32_t, 0x0062fbf4) = edx;

    uint32_t eax = result;
    if (GP4_GLOBAL(uint32_t, 0x00624848) == 0u) {
        eax = edx;
    }
    product = static_cast<int64_t>(static_cast<int32_t>(eax)) * GP4_GLOBAL(int32_t, 0x00603ac0);
    edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    int32_t ecx = static_cast<int32_t>(eax);
    if (ecx < 0) {
        ecx = -ecx;
    }
    if ((car->FlagsE5 & 4u) != 0u && (car->Flags230 & 0x40u) != 0u) {
        eax = GP4_ARRAY(uint32_t, 0x00602b30)[GP4_GLOBAL(uint16_t, 0x007ad89e)];
        product = static_cast<int64_t>(static_cast<int32_t>(eax)) * ecx;
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        ecx = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);
    }

    c.eax = eax;
    c.ecx = static_cast<uint32_t>(ecx);
    c.edx = edx;
    gp4::call_regs(0x00427880, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004154d5, FUN_004154d5, "regs(edi:rec[0x8943a0,0x160,0..25], esi:ptr[0x2a0]:layout=car_0043c677+rec_00427880+car_004154d5) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+walk_00427f3c+state_00427d4b+state_00427880+state_004154d5")
