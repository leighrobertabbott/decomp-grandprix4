#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Two ring-record fields (the 0x160-byte records at 0x8943a0): head and tail index of a singly
// linked chain of 8-byte entries in the table at 0x965a30.
struct Unknown_00555ea4_Record {
    uint8_t Unknown00[0x98];
    uint16_t ChainHead98;
    uint16_t ChainTail9A;
};

// Address of the link word (+6) of entry `index` of the 8-byte table at 0x965a30.
static __declspec(noinline) uint16_t* LinkWordAddress(uint32_t index) {
    return reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(0x00965a36u + index * 8u));
}

// FUNCTION: GP4 0x00555ea4
// pushal wrapper. Appends a new entry (dword eax, word si) at the end of the table at 0x965a30
// (count at 0x965a2c, 999 entries; a full table is an int3 assertion) and links its index onto
// the chain of record ebx: the head +0x98 is set when empty, else the old tail's link word
// (+6) is set; the tail +0x9a is always the new index. popal restores every GPR; the flags are
// those of `inc dword [0x965a2c]` (CF still from `or edx, edx`, i.e. clear).
void __cdecl FUN_00555ea4(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint16_t extra = static_cast<uint16_t>(r->esi);
    Unknown_00555ea4_Record* record =
        reinterpret_cast<Unknown_00555ea4_Record*>(static_cast<uintptr_t>(r->ebx));

    const uint32_t slot = GP4_GLOBAL(uint32_t, 0x00965a2c) * 8u + 0x00965a30u;
    if (!(slot < 0x00967968u)) {
        __debugbreak();
    }
    const uint16_t index = static_cast<uint16_t>((slot - 0x00965a30u) >> 3);

    const uint32_t head = record->ChainHead98;
    if (head == 0u) {
        record->ChainHead98 = index;
        record->ChainTail9A = index;
    } else {
        *LinkWordAddress(record->ChainTail9A) = index;
        record->ChainTail9A = index;
    }
    GP4_FIELD(uint32_t, static_cast<uintptr_t>(slot), 0) = value;
    GP4_FIELD(uint16_t, static_cast<uintptr_t>(slot), 4) = extra;

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00965a2c);
    GP4_GLOBAL(uint32_t, 0x00965a2c) = count + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(count, gp4::flags::logic32(head)));
}
GP4_IMPL(0x00555ea4, FUN_00555ea4, "regs(eax:u32, esi:u32, ebx:ptr[0xa0]:u16{=0,1,2,3,10,500,998}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=idx_00965a2c")
