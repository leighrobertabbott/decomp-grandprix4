"""COM model (gp4re/com.py) and narrow return types (gp4re/retwidth.py) on synthetic x86.

Each pair runs in the real oracle: the original and the reconstruction call a modelled
Direct3D 8 method; a difference in an argument, in an input structure at call time, or in what
is written into a lock buffer must fail, identical code must not.
"""
from __future__ import annotations

import struct
import unittest
from unittest.mock import patch

from gp4re.emu import Impl, differential
from gp4re.spec import parse

from test_emu_soundness import INIT, TARGET, images   # tests/ is on sys.path (rootdir conftest-less)

CFG = {"init_calls": [INIT], "max_cases": 8, "time_budget_s": 2,
       "min_block_coverage": 0.90, "min_branch_coverage": 0.70, "ni_cases": 1}


def run(original: bytes, reconstruction: bytes, spec: str, extra_code: dict[int, bytes] | None = None,
        starts: set[int] | None = None) -> dict:
    gp4, dll = images(original, reconstruction, extra_code)
    with patch("gp4re.config.oracle", side_effect=CFG.__getitem__):
        return differential(gp4, dll, Impl(TARGET, 0x10001000, "synthetic", spec), parse(spec), fpucw=0x037F,
                            cases=4, function_starts=starts or {TARGET, INIT})


def set_render_state(state: int) -> bytes:
    # mov eax,[esp+4]; mov ecx,[eax]; push dword [esp+8]; push state; push eax; call [ecx+0xc8]; ret
    return (bytes.fromhex("8b442404 8b08 ff742408") + bytes([0x6A, state]) + bytes.fromhex("50 ff91c8000000 c3"))


def set_transform(first: float) -> bytes:
    # write first into m[0][0], SetTransform(0x100, m), then overwrite it with 3.0: the final memory
    # is the same for every first, only the matrix the method saw differs
    return (bytes.fromhex("8b442404 8b542408 c702") + struct.pack("<f", first)
            + bytes.fromhex("8b08 52 6800010000 50 ff9194000000 8b542408 c702") + struct.pack("<f", 3.0)
            + bytes.fromhex("c3"))


def lock_and_write(value: int) -> bytes:
    # Lock(0, 16, &p, 0) on the vertex buffer arg; on success *(u32*)p = value; Unlock is not needed
    return (bytes.fromhex("8b442404 83ec04 8b08 6a00 8d542404 52 6a10 6a00 50 ff512c 85c0 7509 8b0424 c700")
            + struct.pack("<I", value) + bytes.fromhex("83c404 c3"))


def ds_lock_and_write(value: int) -> bytes:
    # IDirectSoundBuffer::Lock(0, 16, &p1, &n1, &p2, &n2, 0) on the buffer arg; on success *(u32*)p1 = value
    return (bytes.fromhex("8b442404 83ec10 8b08 6a00 8d542410 52 8d542410 52 8d542410 52 8d542410 52 6a10 6a00 50"
                          " ff512c 85c0 7509 8b0424 c700") + struct.pack("<I", value) + bytes.fromhex("83c410 c3"))


class ComModelTests(unittest.TestCase):
    DEV = "cdecl(ptr[16]:com=IDirect3DDevice8, u32) -> void"

    def test_same_call_sequence_is_not_a_mismatch(self):
        res = run(set_render_state(7), set_render_state(7), self.DEV)
        self.assertNotEqual(res["status"], "fail", res)

    def test_different_argument_fails(self):
        res = run(set_render_state(7), set_render_state(8), self.DEV)
        self.assertEqual(res["status"], "fail", res)
        self.assertIn("SetRenderState", res["mismatch"])

    def test_input_structure_is_compared_at_call_time(self):
        spec = "cdecl(ptr[16]:com=IDirect3DDevice8, ptr[64]:f32) -> void"
        self.assertNotEqual(run(set_transform(1.0), set_transform(1.0), spec)["status"], "fail")
        res = run(set_transform(1.0), set_transform(2.0), spec)
        self.assertEqual(res["status"], "fail", res)
        self.assertIn("SetTransform", res["mismatch"])

    def test_lock_buffer_contents_are_compared(self):
        spec = "cdecl(ptr[16]:com=IDirect3DVertexBuffer8) -> void"
        self.assertNotEqual(run(lock_and_write(0x11), lock_and_write(0x11), spec)["status"], "fail")
        res = run(lock_and_write(0x11), lock_and_write(0x22), spec)
        self.assertEqual(res["status"], "fail", res)

    def test_sound_buffer_lock_contents_are_compared(self):
        spec = "cdecl(ptr[16]:com=IDirectSoundBuffer) -> void"
        self.assertNotEqual(run(ds_lock_and_write(0x11), ds_lock_and_write(0x11), spec)["status"], "fail")
        res = run(ds_lock_and_write(0x11), ds_lock_and_write(0x22), spec)
        self.assertEqual(res["status"], "fail", res)

    def test_unknown_interface_is_a_spec_error(self):
        with self.assertRaises(ValueError):
            parse("cdecl(ptr[16]:com=IDirectPlay8Peer) -> void")


class NarrowReturnTests(unittest.TestCase):
    CALLER = TARGET + 0x100

    def pair(self, caller_reads: bytes) -> dict:
        body = bytes.fromhex("b001 c3")                     # mov al, 1; ret (upper eax bits: the caller's)
        call = b"\xE8" + struct.pack("<i", TARGET - (self.CALLER + 5)) + caller_reads + b"\xC3"
        return run(body, body, "cdecl() -> u8", {self.CALLER: call}, {TARGET, INIT, self.CALLER})

    def test_al_only_caller_passes(self):
        self.assertNotEqual(self.pair(bytes.fromhex("84c0"))["status"], "fail")      # test al, al

    def test_caller_reading_eax_fails(self):
        res = self.pair(bytes.fromhex("85c0"))                                        # test eax, eax
        self.assertEqual(res["status"], "fail", res)
        self.assertIn("reads more of eax", res["reason"])

    def test_i32_still_sees_the_upper_bits(self):
        body = bytes.fromhex("b001 c3")
        res = run(body, bytes.fromhex("b801000000 c3"), "cdecl() -> i32")             # mov eax, 1
        self.assertEqual(res["status"], "fail", res)


if __name__ == "__main__":
    unittest.main()
