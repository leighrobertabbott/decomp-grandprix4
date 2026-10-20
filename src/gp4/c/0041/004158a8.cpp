#include <gp4/gp4.h>

// (a * b) >> n as "imul; shrd eax, edx, n" leaves it in eax (signed 64-bit product, low dword).
static inline uint32_t mulShr(int32_t a, int32_t b, unsigned n) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

// "test v, v; jns; neg v"
static inline int32_t absNeg(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
}

// FUNCTION: GP4 0x004158a8
// esi = the record the three helpers work on. The largest magnitude m of the signed dwords
// 0x00638cd8 / 0x00638cdc / 0x00638ce0 is scaled: 0x00637f6c = (m * [0x0062d28c]) >> 14. The Q-format
// factors are then
//   0x00638d0c = -(((0x0041246e() + [esi+0x16c]) * ((z * 0x00637f6c) >> 23)) >> 14)
//   0x00638d04 = -(((((x * 0x00637f6c) >> 23) * [0x006037ac]) >> 14)
//   0x00638d08 = -(((((y * 0x00637f6c) >> 23) * [0x006037b0]) >> 14)
// with x = [0x638cd8], y = [0x638cdc], z = [0x638ce0]; 0x004124a4 and 0x0040febd (ecx = the
// (z * 0x00637f6c) >> 23 factor) follow and leave eax/edx/ebp/flags.
void __cdecl FUN_004158a8(gp4::Regs* r) {
    int32_t largest = absNeg(GP4_GLOBAL(int32_t, 0x00638cd8));
    const int32_t second = absNeg(GP4_GLOBAL(int32_t, 0x00638cdc));
    const int32_t third = absNeg(GP4_GLOBAL(int32_t, 0x00638ce0));
    if (largest < third) {
        largest = third;
    }
    if (largest < second) {
        largest = second;
    }
    GP4_GLOBAL(uint32_t, 0x00637f6c) = mulShr(largest, GP4_GLOBAL(int32_t, 0x0062d28c), 14);

    r->ecx = mulShr(GP4_GLOBAL(int32_t, 0x00638ce0), static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00637f6c)), 23);
    gp4::call_regs(0x0041246e, *r);
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t sum = r->eax + GP4_FIELD(uint32_t, body, 0x16c);
    GP4_GLOBAL(uint32_t, 0x00638d0c) = 0u - mulShr(static_cast<int32_t>(sum), static_cast<int32_t>(r->ecx), 14);

    const uint32_t x = mulShr(GP4_GLOBAL(int32_t, 0x00638cd8), static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00637f6c)), 23);
    GP4_GLOBAL(uint32_t, 0x00638d04) = 0u - mulShr(static_cast<int32_t>(x), GP4_GLOBAL(int32_t, 0x006037ac), 14);

    const uint32_t y = mulShr(GP4_GLOBAL(int32_t, 0x00638cdc), static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00637f6c)), 23);
    GP4_GLOBAL(uint32_t, 0x00638d08) = 0u - mulShr(static_cast<int32_t>(y), GP4_GLOBAL(int32_t, 0x006037b0), 14);

    gp4::call_regs(0x004124a4, *r);
    gp4::call_regs(0x0040febd, *r);
}
GP4_IMPL(0x004158a8, FUN_004158a8, "regs(esi:ptr[0x310]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041246e")
