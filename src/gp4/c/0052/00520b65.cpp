#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original indexes three dword tables with the constant edx = 2 ([edx*4 + base]); the compiler
// would fold that into absolute addresses and hide the table bases.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// FUNCTION: GP4 0x00520b65
// pushal wrapper that loads game-state slot eax (the counterpart of the saver 0x00520b1b): after switching the
// groups off (0x00441a4f) and rebasing the pointer tables to offsets (0x00513982) it reads the dword at +4 of
// the ring slot (the third ring position: [0x679b3c], or the lower bound [0x679ba4] when it is above the upper
// limit [0x679b2c]) into [0x679b20], copies [0x6975c0] bytes (as dwords) of the requested slot
// (slot * [0x6975c4] + [0x7a8e18]) over the state block 0x0066d810, loads the 0xa5e-dword table 0x0063b814 from
// the bytes behind it (0x0040b526), copies the word [0x66d8ba] to [0x7ad892], converts the pointers back
// (0x005139ed) and runs the setup 0x00441a5c with eax = the end of the slot, edx = the high half of
// slot * pitch and edi = the end of the state block. popal restores every GPR; the flags are those 0x00441a5c leaves.
void __cdecl FUN_00520b65(gp4::Regs* r) {
    const int32_t slotIndex = static_cast<int32_t>(r->eax);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00441a4f, c);
    gp4::call_regs(0x00513982, c);

    int32_t position = tableEntry(GP4_ARRAY(int32_t, 0x00679b34), 2);
    if (position > tableEntry(GP4_ARRAY(int32_t, 0x00679b24), 2)) {
        position = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), 2);
    }
    const int64_t ringOffset = static_cast<int64_t>(position) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006975c4));
    const uint32_t* ringSlot = reinterpret_cast<const uint32_t*>(
        static_cast<uintptr_t>(static_cast<uint32_t>(ringOffset) + GP4_GLOBAL(uint32_t, 0x007a8e18)));
    GP4_GLOBAL(uint32_t, 0x00679b20) = ringSlot[1];

    const int64_t slotOffset = static_cast<int64_t>(slotIndex) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006975c4));
    const uint32_t* src = reinterpret_cast<const uint32_t*>(
        static_cast<uintptr_t>(static_cast<uint32_t>(slotOffset) + GP4_GLOBAL(uint32_t, 0x007a8e18)));
    uint32_t* dst = GP4_ARRAY(uint32_t, 0x0066d810);
    for (uint32_t left = GP4_GLOBAL(uint32_t, 0x006975c0) >> 2; left != 0u; --left) {
        *dst++ = *src++;
    }

    c.eax = static_cast<uint32_t>(slotOffset);
    c.ecx = 0u;
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(slotOffset) >> 32);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    gp4::call_regs(0x0040b526, c);
    c.esi = c.eax;

    GP4_GLOBAL(uint16_t, 0x007ad892) = GP4_GLOBAL(uint16_t, 0x0066d8ba);
    gp4::call_regs(0x005139ed, c);
    gp4::call_regs(0x00441a5c, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00520b65, FUN_00520b65, "regs(eax:u32[0..3], edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+idx_00520b65")
