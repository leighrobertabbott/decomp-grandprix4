#include <gp4/gp4.h>

// Pushes one key word onto the 64-word key ring at 0x008245c8 (write cursor [0x008245c0], read
// cursor [0x008245c4]; the reader is 0x0052d7e2) when the mode byte 0x00824648 is 0xff. Bit 1 of
// the byte 0x008245a2 sets bit 15 of the stored word. The cursor wraps to the ring start at
// 0x00824648; when the advanced cursor would meet the read cursor the queue is full and the word
// is dropped.
// FUNCTION: GP4 0x0052d777
void __stdcall FUN_0052d777(int key) {
    if (GP4_GLOBAL(uint8_t, 0x00824648) != 0xffu) {
        return;
    }
    uint32_t word = static_cast<uint16_t>(key);
    if ((GP4_GLOBAL(uint8_t, 0x008245a2) & 2u) != 0u) {
        word |= 0x8000u;
    }
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008245c0);
    uint32_t next = cursor + 2u;
    if (next >= 0x00824648u) {
        next = 0x008245c8u;
    }
    if (next != GP4_GLOBAL(uint32_t, 0x008245c4)) {
        *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(cursor)) = static_cast<uint16_t>(word);
        GP4_GLOBAL(uint32_t, 0x008245c0) = next;
    }
}
GP4_IMPL(0x0052d777, FUN_0052d777, "stdcall(i32) -> void; globals=fuzz:i32+layout=keyq_008245c0")
