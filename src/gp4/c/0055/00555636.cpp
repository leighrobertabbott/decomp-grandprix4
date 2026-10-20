#include <gp4/gp4.h>
#include <gp4/flags.h>

// edi = record. Rotates the record's first vector (0x0055671d; [0x934df8] = 0x28 first) and keeps
// the raw point at +0x118/+0x11c; with the point scaled down by 32 the words +0xe0/+0xb8/+0xc0 get
// (y - [+0xf4])/32/8, (x - [+0xf0]/32)/8 and (z - [+0xf8]/32)/8 and 0x8754ac/0x8754b4 the point.
// Bit 0x200 of word +0x48 picks the second vector (0x00553ab7, else 0x00553aef); scaled by 32 it is
// subtracted from 0x8754ac/0x8754b4. eax = isqrt(dx*dx + dz*dz) / 8 (0x0040113f); the angle of
// (dx, dz) (0x00401315) relative to word +0 indexes the Q14 table 0x7c5c5c and ebp = 0x10000000 /
// max(|value|, 0x1000) (edx = remainder). cl = 0x20 on return; flags of `cmp ebp, 0x1000`.
static __declspec(noinline) void Call553ab7(gp4::Regs* r) { gp4::call_regs(0x00553ab7, *r); }
static __declspec(noinline) void Call553aef(gp4::Regs* r) { gp4::call_regs(0x00553aef, *r); }

// FUNCTION: GP4 0x00555636
void __cdecl FUN_00555636(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t savedEcx = (r->ecx & ~0xffu) | 0x20u;
    r->ecx = savedEcx;
    GP4_GLOBAL(uint32_t, 0x00934df8) = 0x28u;
    gp4::call_regs(0x0055671d, *r);

    int32_t& pointX = GP4_GLOBAL(int32_t, 0x00934df0);
    int32_t& pointZ = GP4_GLOBAL(int32_t, 0x00934df4);
    GP4_FIELD(int32_t, rec, 0x118) = pointX;
    GP4_FIELD(int32_t, rec, 0x11c) = pointZ;
    pointX >>= 5;
    pointZ >>= 5;
    const int32_t height = static_cast<int32_t>(r->ecx - GP4_FIELD(uint32_t, rec, 0xf4)) >> 5;
    r->ecx = savedEcx;
    GP4_FIELD(uint16_t, rec, 0xe0) = static_cast<uint16_t>(height >> 3);

    const int32_t x = pointX;
    GP4_GLOBAL(int32_t, 0x008754ac) = x;
    int32_t base = GP4_FIELD(int32_t, rec, 0xf0) >> 5;
    GP4_FIELD(uint16_t, rec, 0xb8) =
        static_cast<uint16_t>(static_cast<int32_t>(static_cast<uint32_t>(x) - static_cast<uint32_t>(base)) >> 3);
    const int32_t z = pointZ;
    GP4_GLOBAL(int32_t, 0x008754b4) = z;
    base = GP4_FIELD(int32_t, rec, 0xf8) >> 5;
    const int32_t offsetZ = static_cast<int32_t>(static_cast<uint32_t>(z) - static_cast<uint32_t>(base)) >> 3;
    GP4_FIELD(uint16_t, rec, 0xc0) = static_cast<uint16_t>(offsetZ);
    r->eax = static_cast<uint32_t>(offsetZ);
    r->edx = static_cast<uint32_t>(base);

    const uint16_t mode = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x48) & 0x200u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(mode));
    if (mode != 0u) {
        Call553ab7(r);
    } else {
        Call553aef(r);
    }
    pointZ >>= 5;
    pointX >>= 5;
    const uint32_t dx = GP4_GLOBAL(uint32_t, 0x008754ac) - static_cast<uint32_t>(pointX);
    GP4_GLOBAL(uint32_t, 0x008754ac) = dx;
    const int64_t squareX = static_cast<int64_t>(static_cast<int32_t>(dx)) * static_cast<int32_t>(dx);
    const uint32_t dz = GP4_GLOBAL(uint32_t, 0x008754b4) - static_cast<uint32_t>(pointZ);
    GP4_GLOBAL(uint32_t, 0x008754b4) = dz;
    const int64_t squareZ = static_cast<int64_t>(static_cast<int32_t>(dz)) * static_cast<int32_t>(dz);
    const uint64_t sum = static_cast<uint64_t>(squareX) + static_cast<uint64_t>(squareZ);
    r->ebp = static_cast<uint32_t>(static_cast<uint64_t>(squareX) >> 32);
    r->eax = static_cast<uint32_t>(sum);
    r->edx = static_cast<uint32_t>(sum >> 32);
    gp4::call_regs(0x0040113f, *r);
    const uint32_t distance = static_cast<uint32_t>(static_cast<int32_t>(r->eax) >> 3);

    r->eax = GP4_GLOBAL(uint32_t, 0x008754ac);
    r->edx = GP4_GLOBAL(uint32_t, 0x008754b4);
    gp4::call_regs(0x00401315, *r);
    uint16_t angle = static_cast<uint16_t>(r->eax - GP4_FIELD(uint16_t, rec, 0));
    angle = static_cast<uint16_t>(0u - angle);
    angle = static_cast<uint16_t>(angle + 0x4000u);
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    angle = static_cast<uint16_t>(angle >> 2);
    const uint32_t offset = static_cast<uint32_t>(angle) & 0xfffeu;
    const int16_t value = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
    int32_t scale = value;
    if (scale < 0) {
        scale = -scale;
    }
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(scale), 0x1000u);
    if (scale < 0x1000) {
        scale = 0x1000;
    }
    const int32_t quotient = 0x10000000 / scale;
    const int32_t remainder = 0x10000000 % scale;
    r->ebp = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eax = distance;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00555636, FUN_00555636, "regs(ecx:u32, edi:ptr[0x158]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:f64+layout=multiply_0040ce7b")
