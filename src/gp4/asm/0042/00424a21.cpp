#include <gp4/gp4.h>
#include <gp4/x87.h>

// Car record in esi: position dwords +0x00/+0x04/+0x08, angle dwords +0x0c/+0x10/+0x14 and the
// current track-segment record pointer +0x1c.
struct Unknown_00424a21_Car {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
    uint8_t Unknown18[0x1c - 0x18];
    uint8_t* Segment1C;
};

// Object record in ebx.
struct Unknown_00424a21_Object {
    int32_t Unknown00[3];
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
    int32_t Unknown18[3];
    int32_t Unknown24[3];
    int32_t Unknown30[3];
    uint8_t Unknown3C[0x58 - 0x3c];
    int32_t Unknown58;
    int32_t Unknown5C;
    uint8_t Unknown60[0x64 - 0x60];
    uint32_t Unknown64;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` (32-bit).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The rotation block the original addresses through edx = 0x00622dc8.
static __declspec(noinline) uint8_t* rotationBlock() {
    return GP4_ARRAY(uint8_t, 0x00622dc8);
}

// fild the three dwords at 0x00623e18/1c/20 into the input vector (+0xb8/+0xc0/+0xc8) and rotate it
// with 0x00401f02; then fistp the rotated vector (+0xa0/+0xa8/+0xb0) into 0x00623e30/34/38.
static inline void rotate(gp4::Regs& c) {
    uint8_t* block = rotationBlock();
    GP4_FIELD(double, block, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e18));
    GP4_FIELD(double, block, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e1c));
    GP4_FIELD(double, block, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e20));
    c.edx = reinterpret_cast<uint32_t>(block);
    gp4::call_regs(0x00401f02, c);
    GP4_GLOBAL(int32_t, 0x00623e30) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e34) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e38) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xb0));
}

// ((random byte from 0x00401462) << 6) * scale, Q14; edx is the product's high dword.
static inline uint32_t randomTerm(gp4::Regs& c, int32_t scale) {
    gp4::call_regs(0x00401462, c);
    const int32_t sample = static_cast<int32_t>((c.eax & 0xffu) << 6);
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(sample) * static_cast<int64_t>(scale));
    c.edx = static_cast<uint32_t>(product >> 32);
    return static_cast<uint32_t>(product >> 14);
}

// FUNCTION: GP4 0x00424a21
// Launches the object record in ebx from the car in esi: copies the car's segment index, angles and
// rotated offset position, takes the segment height (+6 << 8) and the 0x00443db0 projection as the
// base velocity, adds a random rotated launch velocity (coefficients 0x00609884..0x006098a4) and a
// random rotated spin (0x006098a8..0x006098bc, adjusted by 0x004248af).
void __cdecl FUN_00424a21(gp4::Regs* r) {
    Unknown_00424a21_Object* object = reinterpret_cast<Unknown_00424a21_Object*>(static_cast<uintptr_t>(r->ebx));
    const Unknown_00424a21_Car* car = reinterpret_cast<const Unknown_00424a21_Car*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    object->Unknown64 = reinterpret_cast<uint32_t>(car->Segment1C) - 0x008943a0u;
    object->Unknown0C = car->Unknown10;
    object->Unknown10 = car->Unknown0C;
    object->Unknown14 = car->Unknown14;
    c.eax = car->Unknown14;
    gp4::call_regs(0x00401f8e, c);

    GP4_GLOBAL(int32_t, 0x00623e18) = object->Unknown30[0];
    GP4_GLOBAL(int32_t, 0x00623e1c) = object->Unknown30[1];
    GP4_GLOBAL(int32_t, 0x00623e20) = object->Unknown30[2];
    c.eax = static_cast<uint32_t>(object->Unknown30[2]);
    rotate(c);
    object->Unknown00[0] = car->Unknown00 + GP4_GLOBAL(int32_t, 0x00623e30);
    object->Unknown00[1] = car->Unknown08 + GP4_GLOBAL(int32_t, 0x00623e34);
    object->Unknown00[2] = car->Unknown04 + GP4_GLOBAL(int32_t, 0x00623e38);

    const int32_t height = static_cast<int32_t>(GP4_FIELD(int16_t, car->Segment1C, 6)) << 8;
    object->Unknown58 = height;
    c.eax = static_cast<uint32_t>(height);
    gp4::call_regs(0x00443db0, c);
    GP4_GLOBAL(uint32_t, 0x00623e30) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00623e34) = c.edx;
    GP4_GLOBAL(uint32_t, 0x00623e38) = c.ecx;
    object->Unknown18[0] = GP4_GLOBAL(int32_t, 0x00623e30) >> 8;
    object->Unknown18[1] = GP4_GLOBAL(int32_t, 0x00623e34) >> 8;
    object->Unknown18[2] = GP4_GLOBAL(int32_t, 0x00623e38) >> 8;
    c.eax = static_cast<uint32_t>(object->Unknown18[2]);

    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x00609884)) + GP4_GLOBAL(uint32_t, 0x00609888)
             + GP4_GLOBAL(uint32_t, 0x0060988c)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e18) = c.eax;
    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x00609890)) + GP4_GLOBAL(uint32_t, 0x00609894)
             + GP4_GLOBAL(uint32_t, 0x00609898)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = c.eax;
    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x0060989c)) + GP4_GLOBAL(uint32_t, 0x006098a0)
             + GP4_GLOBAL(uint32_t, 0x006098a4)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e20) = c.eax;
    rotate(c);
    object->Unknown18[0] += GP4_GLOBAL(int32_t, 0x00623e30);
    object->Unknown18[1] += GP4_GLOBAL(int32_t, 0x00623e34) + object->Unknown5C;
    object->Unknown18[2] += GP4_GLOBAL(int32_t, 0x00623e38);
    object->Unknown24[0] = 0;
    object->Unknown24[1] = 0;
    object->Unknown24[2] = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00623e38);

    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x006098a8)) + GP4_GLOBAL(uint32_t, 0x006098ac)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e18) = c.eax;
    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x006098b0)) + GP4_GLOBAL(uint32_t, 0x006098b4)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = c.eax;
    c.eax = (randomTerm(c, GP4_GLOBAL(int32_t, 0x006098b8)) + GP4_GLOBAL(uint32_t, 0x006098bc)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e20) = c.eax;
    gp4::call_regs(0x004248af, c);
    rotate(c);
    object->Unknown24[0] += GP4_GLOBAL(int32_t, 0x00623e30);
    object->Unknown24[1] += GP4_GLOBAL(int32_t, 0x00623e34);
    const uint32_t spin = GP4_GLOBAL(uint32_t, 0x00623e38);
    const uint32_t before = static_cast<uint32_t>(object->Unknown24[2]);
    object->Unknown24[2] = static_cast<int32_t>(before + spin);

    r->eax = spin;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->ebx = c.ebx;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, spin);
}
GP4_IMPL(0x00424a21, FUN_00424a21, "regs(ebx:ptr[0x80]:layout=rec_0042453d+rec_00424878, esi:ptr[0x41c]:layout=car_0041e500+car_00424878) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_0066d8c0+state_0041bce9+state_00424a21")
