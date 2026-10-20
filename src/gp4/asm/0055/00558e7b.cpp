#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00558e7b
// Called eight times by 0x00558ece with ebx = a word offset into a 0x60-byte entry of the table at
// 0x9559f8 (0xb..0xe, 0x14..0x17) and ebp = the matching attribute bit (0x800 .. 0x40000) of the
// dword at edi+0x2c; ecx accumulates the bits. Sets ecx |= ebp when the record does not have the
// bit, when the entry word leads (via the 10-byte table at 0x961b9c and the word slots at
// 0x9e1b20) to an empty slot (0xffff), or when the slot's 0x20-byte entry of the table at
// 0x6975ec has bit 3 set in its word +0x14. eax and ebx are left holding the intermediate lookups.
void __cdecl FUN_00558e7b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t bit = r->ebp;
    uint32_t ecx = r->ecx;
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t flags;

    if ((GP4_FIELD(uint32_t, record, 0x2c) & bit) == 0u) {
        ecx |= bit;
        flags = gp4::flags::logic32(ecx);
    } else {
        const uint32_t index = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x56)) * 0x30u + ebx;
        const uint32_t entry = GP4_ARRAY(uint16_t, 0x009559f8)[index];
        ebx = entry * 10u + 0x00961b9cu;
        eax = GP4_GLOBAL(uint16_t, ebx);
        const uint32_t slot = GP4_ARRAY(uint16_t, 0x009e1b20)[eax];
        if (slot == 0xffffu) {
            ecx |= bit;
            flags = gp4::flags::logic32(ecx);
        } else {
            eax = slot + 0x006975ecu;
            const uint32_t attr = GP4_GLOBAL(uint16_t, eax + 0x14u) & 8u;
            flags = gp4::flags::logic16(attr);
            if (attr != 0u) {
                ecx |= bit;
                flags = gp4::flags::logic32(ecx);
            }
        }
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558e7b, FUN_00558e7b, "regs(edi:ptr[0x60]:u16[0..7], ebp:u32{0x10000,0x20000,0x40000}, ebx:u32[0xb..0x17], ecx:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0051cc50")
