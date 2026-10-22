#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d9080 { uint8_t Unknown00[0x2d4]; };
#pragma pack(pop)

// Builds a bit mask over the 25 slots (0x2a0 bytes apart) of the object at table entry `index` of
// 0x00a51fd8: bit k is set when 0x0048e020(slot k) returns non-zero and the byte at +0x2d4 + k * 0x2a0
// of the object is non-zero.
// FUNCTION: GP4 0x004d9080
uint32_t __stdcall FUN_004d9080(int32_t index) {
    uint32_t mask = 0;
    uint32_t bit = 1;
    int32_t offset = 0;
    int32_t slot = 0;
    do {
        Unknown_004d9080* object = GP4_ARRAY(Unknown_004d9080*, 0x00a51fd8)[index];
        if (gp4::thiscall_<0x0048e020, uint8_t>(object, slot) != 0) {
            const uint8_t* flags = (const uint8_t*)GP4_ARRAY(Unknown_004d9080*, 0x00a51fd8)[index] + offset;
            if (flags[0x2d4] != 0) {
                mask |= bit;
            }
        }
        bit += bit;
        offset += 0x2a0;
        slot++;
    } while (offset < 0x41a0);
    return mask;
}
GP4_IMPL(0x004d9080, FUN_004d9080, "stdcall(i32[0..3]) -> u32; gfields={0x00a51fd8*4/4:ptr[0x4400]:{0x2d4*25/0x2a0:u8{=0,1,0xff}, 0x414*24/0x2a0:u8{=0,1,1,0xff}, 0x408:u32{=0}, 0x6a8:u32[0..1], 0x948:u32[0..2], 0xbe8:u32[0..3], 0xe88:u32[0..4], 0x1128:u32[0..5], 0x13c8:u32[0..6], 0x1668:u32[0..7], 0x1908:u32[0..8], 0x1ba8:u32[0..9], 0x1e48:u32[0..10], 0x20e8:u32[0..11], 0x2388:u32[0..12], 0x2628:u32[0..13], 0x28c8:u32[0..14], 0x2b68:u32[0..15], 0x2e08:u32[0..16], 0x30a8:u32[0..17], 0x3348:u32[0..18], 0x35e8:u32[0..19], 0x3888:u32[0..20], 0x3b28:u32[0..21], 0x3dc8:u32[0..22], 0x4068:u32[0..23]}}")
