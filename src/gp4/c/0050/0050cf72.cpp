#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written helper: looks up the entry selected by the word at 0x009e239c (0xffff = none: cmp
// flags with CF forced to 1, nothing else touched). Otherwise stores the signed byte table entry
// [0x697609 + idx] * 0x100 + 0x300 in 0x009e7d94, stores
//   (((int16)[edi+0x176] + (int16)[edi+0x16]) >> 1) * [0x0073d198] >> 14
// in 0x00948b58 (edx keeps the high half of the product) and loads edi from the dword table entry
// [0x6975ec + idx]; the flags are those of the shrd with CF cleared (clc).
// FUNCTION: GP4 0x0050cf72
void __cdecl FUN_0050cf72(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint16_t, 0x009e239c);
    if (index == 0xffffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, 0xffffu) | gp4::flags::CF);
        return;
    }

    const int32_t entry = GP4_ARRAY(int8_t, 0x00697609)[index];
    GP4_GLOBAL(int32_t, 0x009e7d94) = entry * 0x100 + 0x300;
    const uint32_t nextEdi = GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x006975ec), index);

    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t sum = static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x176))
                      + static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x16));
    const int32_t half = sum >> 1;
    const int64_t product = static_cast<int64_t>(half) * GP4_GLOBAL(int32_t, 0x0073d198);
    const uint64_t bits = static_cast<uint64_t>(product);
    const uint32_t low = static_cast<uint32_t>(bits >> 14);
    GP4_GLOBAL(uint32_t, 0x00948b58) = low;

    // shrd eax, edx, 14 then clc: ZF/SF/PF from the result, AF = 0 (pinned oracle model),
    // OF = bit31 of ((edx:eax >> 13) ^ result), CF = 0.
    const uint32_t previous = static_cast<uint32_t>(bits >> 13);
    uint32_t arith = gp4::flags::szp(low, 32);
    if (((previous ^ low) & 0x80000000u) != 0u) {
        arith |= gp4::flags::OF;
    }

    r->eax = low;
    r->edx = static_cast<uint32_t>(bits >> 32);
    r->edi = nextEdi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0050cf72, FUN_0050cf72, "regs(eax:u32, edx:u32, edi:ptr[0x180]:i16[-3000..3000]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32; gfields={0x009e239c:u32{0xffff,0,1,2,3,5,9,17}}")
