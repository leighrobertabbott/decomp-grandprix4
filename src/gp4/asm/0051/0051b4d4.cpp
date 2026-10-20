#include <gp4/gp4.h>
#include <gp4/flags.h>

// One gradient record of the palette script walked by 0x0051b458 (esi = record, 8 bytes):
// count, then the start/end bytes of three 8.8 components. Each step is the 16-bit difference
// (end - start) << 8 divided (idiv bp) by max(count - 1, 1) and stored in 0x00933a24/26/28; the
// middle component's start and step are scaled by the Q14 factor [0x009328dc] (imul, bits 14..45).
// `count` entries of three words are written from 0x007c535c + 6 * ([0x00933a62] & 0xff), each
// component advancing by its step, and [0x00933a62] counts the entries. Leaves esi past the
// record, ecx = 0, edi past the last entry, the components in ax/bx/dx (upper halves: eax and edx
// from the last scale product, ebx unchanged); the flags are those of the final `add edi, 6`.
// FUNCTION: GP4 0x0051b4d4
void __cdecl FUN_0051b4d4(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t count = rec[0];
    int32_t steps = static_cast<int32_t>(count) - 1;
    if (steps < 1) {
        steps = 1;
    }
    const int16_t divisor = static_cast<int16_t>(steps);

    const uint16_t first0 = static_cast<uint16_t>(rec[1] << 8);
    const int16_t firstDiff = static_cast<int16_t>(static_cast<uint16_t>((rec[2] << 8) - first0));
    GP4_GLOBAL(int16_t, 0x00933a24) = static_cast<int16_t>(firstDiff / divisor);

    const uint16_t second0Raw = static_cast<uint16_t>(rec[3] << 8);
    const int16_t secondDiff = static_cast<int16_t>(static_cast<uint16_t>((rec[4] << 8) - second0Raw));
    const int16_t secondStepRaw = static_cast<int16_t>(secondDiff / divisor);
    const int64_t stepProduct = static_cast<int64_t>(secondStepRaw) *
                                static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x009328dc));
    GP4_GLOBAL(uint16_t, 0x00933a26) = static_cast<uint16_t>(static_cast<uint32_t>(stepProduct >> 14));
    const int64_t startProduct = static_cast<int64_t>(static_cast<int16_t>(second0Raw)) *
                                 static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x009328dc));
    const uint32_t startScaled = static_cast<uint32_t>(startProduct >> 14);
    const uint32_t startHigh = static_cast<uint32_t>(startProduct >> 32);

    const uint16_t third0 = static_cast<uint16_t>(rec[5] << 8);
    const int16_t thirdDiff = static_cast<int16_t>(static_cast<uint16_t>((rec[6] << 8) - third0));
    GP4_GLOBAL(int16_t, 0x00933a28) = static_cast<int16_t>(thirdDiff / divisor);

    uint16_t* entry = GP4_ARRAY(uint16_t, 0x007c535c) + (GP4_GLOBAL(uint16_t, 0x00933a62) & 0xffu) * 3u;
    uint16_t first = first0;
    uint16_t second = static_cast<uint16_t>(startScaled);
    uint16_t third = third0;
    uint32_t left = count;
    do {
        entry[0] = first;
        entry[1] = second;
        entry[2] = third;
        first = static_cast<uint16_t>(first + GP4_GLOBAL(uint16_t, 0x00933a24));
        second = static_cast<uint16_t>(second + GP4_GLOBAL(uint16_t, 0x00933a26));
        third = static_cast<uint16_t>(third + GP4_GLOBAL(uint16_t, 0x00933a28));
        GP4_GLOBAL(uint16_t, 0x00933a62) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a62) + 1u);
        entry += 3;
        --left;
    } while (left != 0u);

    const uint32_t end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->eax = (startScaled & 0xffff0000u) | first;
    r->ebx = (r->ebx & 0xffff0000u) | second;
    r->ecx = 0u;
    r->edx = (startHigh & 0xffff0000u) | third;
    r->esi += 8u;
    r->edi = end;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(end - 6u, 6u));
}
GP4_IMPL(0x0051b4d4, FUN_0051b4d4, "regs(esi:ptr[0x10]:u8[1..255], ebx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
