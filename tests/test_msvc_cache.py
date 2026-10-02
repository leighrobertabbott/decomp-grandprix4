import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from threading import Barrier

from gp4re.msvc import _publish_object


class ObjectCacheTests(unittest.TestCase):
    def test_simultaneous_publication_keeps_complete_object_and_removes_temps(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            obj = root / "cached.obj"
            payload = bytes(range(256)) * 100
            temps = [root / f"compile-{i}.obj" for i in range(8)]
            for tmp in temps:
                tmp.write_bytes(payload)
            barrier = Barrier(len(temps))

            def publish(tmp):
                barrier.wait()
                _publish_object(tmp, obj)

            with ThreadPoolExecutor(max_workers=len(temps)) as pool:
                list(pool.map(publish, temps))
            self.assertEqual(obj.read_bytes(), payload)
            self.assertFalse(any(tmp.exists() for tmp in temps))

    def test_existing_open_cache_entry_is_never_replaced(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            obj, tmp = root / "cached.obj", root / "candidate.obj"
            obj.write_bytes(b"published")
            tmp.write_bytes(b"candidate")
            with obj.open("rb") as reader:
                _publish_object(tmp, obj)
                self.assertEqual(reader.read(), b"published")
            self.assertEqual(obj.read_bytes(), b"published")
            self.assertFalse(tmp.exists())


if __name__ == "__main__":
    unittest.main()
