#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Slot_0046a6b0 { uint8_t Unknown00[0x20]; uint8_t Flag20; uint8_t Unknown21[0x2b]; };
struct SlotTable_0046a6b0 { uint8_t Unknown00[0x24]; Slot_0046a6b0* Slots24; };
struct SlotOwner_0046a6b0 { uint32_t Unknown00; SlotTable_0046a6b0* Table04; };
struct IndexMap_0046a6b0 { uint8_t Unknown00[0x20c]; const uint32_t* Map20c; };
struct IndexOwner_0046a6b0 { uint32_t Unknown00; IndexMap_0046a6b0* Map04; };
#pragma pack(pop)

// Looks up the slot index for *pIndex in the index map of the second object (0xffffffff = none) and
// returns whether that slot's flag byte (+0x20 of a 0x4c-byte slot of the first object) is non-zero.
// FUNCTION: GP4 0x0046a6b0
uint8_t __stdcall FUN_0046a6b0(SlotOwner_0046a6b0* a, IndexOwner_0046a6b0* b, const uint32_t* pIndex) {
    uint32_t index = *pIndex;
    if (index != 0xffffffffu) {
        uint32_t slot = b->Map04->Map20c[index];
        if (slot != 0xffffffffu) {
            if (a->Table04->Slots24[slot].Flag20 != 0) {
                return 1;
            }
        }
    }
    return 0;
}
GP4_IMPL(0x0046a6b0, FUN_0046a6b0, "stdcall(ptr[8]:{4:ptr[0x30]:{0x24:ptr[0x140]:{0x20*4/0x4c:u8{0,1,0x80}}}}, ptr[8]:{4:ptr[0x214]:{0x20c:ptr[0x20]:u32{=0xffffffff,0,1,2,3}}}, ptr[4]:u32{=0xffffffff,0,1,2,3,4,5,6,7}) -> u8")
