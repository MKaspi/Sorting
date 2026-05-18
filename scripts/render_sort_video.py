#!/usr/bin/env python3
# ASCII-only source on purpose.
import argparse
import math
import mmap
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from typing import Optional, Tuple

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Missing dependency: pillow. Install with: python3 -m pip install pillow", file=sys.stderr)
    sys.exit(2)


I64 = struct.Struct("<q")
NONE = -(2 ** 63)


@dataclass
class Event:
    kind: str
    a: int
    b: int
    direction: str = ""


class Progress:
    def __init__(self, every_seconds: float):
        self.every_seconds = every_seconds
        self.start = time.monotonic()
        self.last = self.start

    def log(self, message: str, force: bool = False) -> None:
        now = time.monotonic()
        if force or self.every_seconds <= 0 or now - self.last >= self.every_seconds:
            elapsed = now - self.start
            print("[{elapsed:8.1f}s] {msg}".format(elapsed=elapsed, msg=message), flush=True)
            self.last = now


def human_bytes(n: int) -> str:
    units = ["B", "KiB", "MiB", "GiB", "TiB"]
    v = float(n)
    for u in units:
        if abs(v) < 1024.0 or u == units[-1]:
            return "{:.2f} {}".format(v, u)
        v /= 1024.0
    return "{} B".format(n)


def parse_event(line: bytes) -> Optional[Event]:
    p = line.strip().split()
    if not p:
        return None
    if p[0] == b"compare" and len(p) >= 3:
        return Event("compare", int(p[1]), int(p[2]))
    if p[0] == b"swap" and len(p) >= 3:
        return Event("swap", int(p[1]), int(p[2]))
    if p[0] == b"copy" and len(p) >= 4:
        return Event("copy", int(p[2]), int(p[3]), p[1].decode("ascii", "replace"))
    return None


class MMapArray:
    def __init__(self, path: str, n: int):
        self.path = path
        self.n = n
        self.file = open(path, "r+b")
        self.mm = mmap.mmap(self.file.fileno(), 0)

    def get(self, idx: int) -> Optional[int]:
        v = I64.unpack_from(self.mm, idx * 8)[0]
        if v == NONE:
            return None
        return v

    def set(self, idx: int, val: Optional[int]) -> None:
        I64.pack_into(self.mm, idx * 8, NONE if val is None else val)

    def close(self) -> None:
        self.mm.flush()
        self.mm.close()
        self.file.close()


def write_i64(f, value: int) -> None:
    f.write(I64.pack(value))


def parse_initial_line_to_files(log_file, main_path: str, aux_path: str, progress: Progress) -> Tuple[int, int, int]:
    # Parses the first line without keeping it in RAM. The log file is left
    # positioned right after the newline. Operation lines are then read
    # sequentially by the main loop.
    n = 0
    vmin = None
    vmax = None
    token = bytearray()

    progress.log("parsing initial array", force=True)

    with open(main_path, "wb") as main_f:
        while True:
            chunk = log_file.read(1024 * 1024)
            if not chunk:
                if token:
                    value = int(token)
                    write_i64(main_f, value)
                    n += 1
                    vmin = value if vmin is None else min(vmin, value)
                    vmax = value if vmax is None else max(vmax, value)
                break

            nl = chunk.find(b"\n")
            if nl >= 0:
                before = chunk[:nl]
                after = chunk[nl + 1:]
                if after:
                    log_file.seek(-len(after), os.SEEK_CUR)

                parts = before.split()
                if token:
                    if parts:
                        token.extend(parts[0])
                        value = int(token)
                        write_i64(main_f, value)
                        n += 1
                        vmin = value if vmin is None else min(vmin, value)
                        vmax = value if vmax is None else max(vmax, value)
                        parts = parts[1:]
                    else:
                        value = int(token)
                        write_i64(main_f, value)
                        n += 1
                        vmin = value if vmin is None else min(vmin, value)
                        vmax = value if vmax is None else max(vmax, value)
                    token = bytearray()

                for part in parts:
                    value = int(part)
                    write_i64(main_f, value)
                    n += 1
                    vmin = value if vmin is None else min(vmin, value)
                    vmax = value if vmax is None else max(vmax, value)

                break

            parts = chunk.split()
            if not parts:
                continue

            if token:
                token.extend(parts[0])
                value = int(token)
                write_i64(main_f, value)
                n += 1
                vmin = value if vmin is None else min(vmin, value)
                vmax = value if vmax is None else max(vmax, value)
                token = bytearray()
                parts = parts[1:]

            if chunk[-1:] not in b" \t\r\n":
                tail = parts[-1]
                parts = parts[:-1]
                token.extend(tail)

            for part in parts:
                value = int(part)
                write_i64(main_f, value)
                n += 1
                vmin = value if vmin is None else min(vmin, value)
                vmax = value if vmax is None else max(vmax, value)

            progress.log("initial elements parsed: {:,}".format(n))

    if n == 0:
        raise RuntimeError("First line must contain initial array values.")

    progress.log("creating aux array: {}".format(human_bytes(n * 8)), force=True)
    with open(aux_path, "wb") as aux_f:
        block_items = 1024 * 1024
        block = I64.pack(NONE) * block_items
        remaining = n
        while remaining > 0:
            take = min(block_items, remaining)
            aux_f.write(block if take == block_items else I64.pack(NONE) * take)
            remaining -= take

    progress.log(
        "initial array done: n={:,}, main={}, aux={}".format(n, human_bytes(n * 8), human_bytes(n * 8)),
        force=True,
    )
    return n, int(vmin), int(vmax)


class BucketView:
    def __init__(self, n: int, pixel_width: int, vmin: int, vmax: int):
        self.n = n
        self.pixel_width = max(1, pixel_width)
        self.bucket_count = self.pixel_width
        self.vmin = vmin
        self.vmax = vmax if vmax != vmin else vmin + 1
        self.value_sum = [0.0] * self.bucket_count
        self.value_count = [0] * self.bucket_count
        self.hot_count = [0.0] * self.bucket_count

    def bucket_of(self, idx: int) -> int:
        if idx < 0:
            return 0
        if idx >= self.n:
            return self.bucket_count - 1
        return min(self.bucket_count - 1, (idx * self.bucket_count) // max(1, self.n))

    def init_from_array(self, arr: MMapArray, progress: Progress) -> None:
        progress.log("building initial buckets", force=True)
        for i in range(arr.n):
            v = arr.get(i)
            if v is not None:
                b = self.bucket_of(i)
                self.value_sum[b] += v
                self.value_count[b] += 1
            if i and i % 1000000 == 0:
                progress.log("bucketed elements: {:,}/{:,}".format(i, arr.n))
        progress.log("initial buckets done", force=True)

    def update_value(self, idx: int, old: Optional[int], new: Optional[int]) -> None:
        b = self.bucket_of(idx)
        if old is not None:
            self.value_sum[b] -= old
            self.value_count[b] -= 1
        if new is not None:
            self.value_sum[b] += new
            self.value_count[b] += 1

    def mark_compare(self, i: int, j: int) -> None:
        self.hot_count[self.bucket_of(i)] += 1.0
        self.hot_count[self.bucket_of(j)] += 1.0

    def mark_write(self, i: int) -> None:
        self.hot_count[self.bucket_of(i)] += 3.0

    def clear_activity(self) -> None:
        for i in range(self.bucket_count):
            self.hot_count[i] = 0.0

    def draw_values(self, draw, box, label, font) -> None:
        x0, y0, x1, y1 = box
        draw.rectangle(box, fill=(18, 18, 18))
        draw.text((x0 + 8, y0 + 6), label, fill=(230, 230, 230), font=font)
        inner_top = y0 + 28
        w = x1 - x0
        for px in range(w):
            b = min(self.bucket_count - 1, (px * self.bucket_count) // max(1, w))
            if self.value_count[b] <= 0:
                color = (35, 35, 35)
            else:
                avg = self.value_sum[b] / max(1, self.value_count[b])
                t = max(0.0, min(1.0, (avg - self.vmin) / (self.vmax - self.vmin)))
                v = int(35 + 210 * t)
                color = (v, v, v)
            draw.line((x0 + px, inner_top, x0 + px, y1 - 4), fill=color)

    def draw_activity(self, draw, box, label, font) -> None:
        x0, y0, x1, y1 = box
        draw.rectangle(box, fill=(12, 12, 12))
        draw.text((x0 + 8, y0 + 6), label, fill=(230, 230, 230), font=font)
        inner_top = y0 + 28
        w = x1 - x0
        local_max = max(max(self.hot_count), 1.0)
        for px in range(w):
            b = min(self.bucket_count - 1, (px * self.bucket_count) // max(1, w))
            t = max(0.0, min(1.0, self.hot_count[b] / local_max))
            v = int(25 + 230 * t)
            draw.line((x0 + px, inner_top, x0 + px, y1 - 4), fill=(v, v, v))


class Renderer:
    def __init__(self, args):
        self.args = args
        self.steps = 0
        self.compares = 0
        self.swaps = 0
        self.copies = 0
        self.unknown = 0
        self.frame_no = 0
        self.font = ImageFont.load_default()
        self.progress = Progress(args.progress_every_seconds)

    def run(self) -> None:
        if not self.args.no_video and not shutil.which("ffmpeg"):
            raise RuntimeError("ffmpeg not found. Install it, for example: sudo apt install ffmpeg")

        if self.args.tmp_dir:
            workdir = os.path.abspath(self.args.tmp_dir)
            os.makedirs(workdir, exist_ok=True)
        else:
            workdir = tempfile.mkdtemp(prefix="sort_video_")

        frames_dir = os.path.join(workdir, "frames")
        if self.args.clean_tmp and os.path.isdir(frames_dir):
            shutil.rmtree(frames_dir)
        os.makedirs(frames_dir, exist_ok=True)
        main_path = os.path.join(workdir, "main.i64")
        aux_path = os.path.join(workdir, "aux.i64")

        self.progress.log("workdir: {}".format(workdir), force=True)

        try:
            file_size = os.path.getsize(self.args.log)
            with open(self.args.log, "rb") as f:
                n, vmin, vmax = parse_initial_line_to_files(f, main_path, aux_path, self.progress)
                main = MMapArray(main_path, n)
                aux = MMapArray(aux_path, n)

                main_view = BucketView(n, self.args.width - 80, vmin, vmax)
                aux_view = BucketView(n, self.args.width - 80, vmin, vmax)
                main_view.init_from_array(main, self.progress)

                self.write_frame(frames_dir, main_view, aux_view, n, "initial")

                self.progress.log("processing operation stream", force=True)
                last_report_ops = 0
                last_report_time = time.monotonic()

                for line in f:
                    ev = parse_event(line)
                    if ev is None:
                        self.unknown += 1
                        continue

                    self.apply_event(ev, main, aux, main_view, aux_view)
                    self.steps += 1

                    if self.steps % self.args.operations_to_frame == 0:
                        self.write_frame(frames_dir, main_view, aux_view, n, ev.kind)
                        main_view.clear_activity()
                        aux_view.clear_activity()

                        if self.args.frames > 0 and self.frame_no >= self.args.frames:
                            self.progress.log("frame limit reached: {:,}".format(self.args.frames), force=True)
                            break

                    now = time.monotonic()
                    if self.args.progress_every_seconds > 0 and now - self.progress.last >= self.args.progress_every_seconds:
                        pos = f.tell()
                        dt = max(0.001, now - last_report_time)
                        dops = self.steps - last_report_ops
                        ops_sec = dops / dt
                        pct = 100.0 * pos / max(1, file_size)
                        self.progress.log(
                            "log read {}/{} ({:.2f}%), operations {:,}, frames {:,}, ops/sec {:.1f}, compare {:,}, swap {:,}, copy {:,}".format(
                                human_bytes(pos), human_bytes(file_size), pct, self.steps, self.frame_no, ops_sec,
                                self.compares, self.swaps, self.copies
                            ),
                            force=True,
                        )
                        last_report_ops = self.steps
                        last_report_time = now

                self.write_frame(frames_dir, main_view, aux_view, n, "final")
                main.close()
                aux.close()

            self.progress.log(
                "log parsed: operations {:,}, compare {:,}, swap {:,}, copy {:,}, unknown {:,}, frames {:,}".format(
                    self.steps, self.compares, self.swaps, self.copies, self.unknown, self.frame_no
                ),
                force=True,
            )

            if self.args.no_video:
                self.progress.log("video skipped, frames in: {}".format(frames_dir), force=True)
            else:
                self.encode(frames_dir)

            if self.args.keep_tmp or self.args.no_video or self.args.tmp_dir:
                self.progress.log("temporary files kept in: {}".format(workdir), force=True)
            else:
                shutil.rmtree(workdir, ignore_errors=True)

        except Exception:
            if self.args.keep_tmp or self.args.tmp_dir:
                print("Temporary files kept in: {}".format(workdir), file=sys.stderr)
            else:
                shutil.rmtree(workdir, ignore_errors=True)
            raise

    def apply_event(self, ev: Event, main: MMapArray, aux: MMapArray, main_view: BucketView, aux_view: BucketView) -> None:
        if ev.kind == "compare":
            self.compares += 1
            main_view.mark_compare(ev.a, ev.b)
            return

        if ev.kind == "swap":
            self.swaps += 1
            if 0 <= ev.a < main.n and 0 <= ev.b < main.n:
                old_a = main.get(ev.a)
                old_b = main.get(ev.b)
                main.set(ev.a, old_b)
                main.set(ev.b, old_a)
                main_view.update_value(ev.a, old_a, old_b)
                main_view.update_value(ev.b, old_b, old_a)
                main_view.mark_write(ev.a)
                main_view.mark_write(ev.b)
            return

        if ev.kind == "copy":
            self.copies += 1
            if ev.direction == "main_to_aux":
                src = ev.a
                dst = ev.b
                if 0 <= src < main.n and 0 <= dst < aux.n:
                    old = aux.get(dst)
                    new = main.get(src)
                    aux.set(dst, new)
                    aux_view.update_value(dst, old, new)
                    aux_view.mark_write(dst)
            elif ev.direction == "aux_to_main":
                src = ev.a
                dst = ev.b
                if 0 <= src < aux.n and 0 <= dst < main.n:
                    old = main.get(dst)
                    new = aux.get(src)
                    main.set(dst, new)
                    main_view.update_value(dst, old, new)
                    main_view.mark_write(dst)

    def write_frame(self, frames_dir: str, main_view: BucketView, aux_view: BucketView, n: int, phase: str) -> None:
        img = Image.new("RGB", (self.args.width, self.args.height), (6, 6, 6))
        draw = ImageDraw.Draw(img)

        margin = 40
        top_h = 78
        gap = 18
        available_h = self.args.height - top_h - margin - gap
        main_h = int(available_h * 0.75)
        aux_h = max(40, available_h - main_h)
        x0 = margin
        x1 = self.args.width - margin
        title = self.args.title or os.path.basename(self.args.log)

        draw.text((margin, 18), title, fill=(245, 245, 245), font=self.font)
        stats = (
            "n={n:,}  operations_to_frame={otf:,}  op={op:,}  compare={cmp:,}  swap={swp:,}  copy={cpy:,}  phase={phase}"
            .format(n=n, otf=self.args.operations_to_frame, op=self.steps, cmp=self.compares,
                    swp=self.swaps, cpy=self.copies, phase=phase)
        )
        draw.text((margin, 42), stats, fill=(220, 220, 220), font=self.font)

        y = top_h
        main_view.draw_values(draw, (x0, y, x1, y + main_h), "main values", self.font)
        y += main_h + gap
        aux_view.draw_values(draw, (x0, y, x1, y + aux_h), "aux values", self.font)

        path = os.path.join(frames_dir, "frame_%08d.png" % self.frame_no)
        img.save(path)
        self.frame_no += 1

    def encode(self, frames_dir: str) -> None:
        ffmpeg = shutil.which("ffmpeg")
        self.progress.log("encoding video with ffmpeg", force=True)
        cmd = [
            ffmpeg,
            "-y",
            "-framerate",
            str(self.args.fps),
            "-i",
            os.path.join(frames_dir, "frame_%08d.png"),
            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            "-movflags",
            "+faststart",
            self.args.out,
        ]
        if self.args.quiet_ffmpeg:
            subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            subprocess.run(cmd, check=True)
        self.progress.log("video written: {}".format(self.args.out), force=True)


def main() -> int:
    ap = argparse.ArgumentParser(description="Render sorting text log to MP4 video using disk-backed arrays.")
    ap.add_argument("log", help="Input text log.")
    ap.add_argument("-o", "--out", default="sort.mp4", help="Output MP4 file.")
    ap.add_argument("--width", type=int, default=1920, help="Video width.")
    ap.add_argument("--height", type=int, default=1080, help="Video height.")
    ap.add_argument("--fps", type=int, default=30, help="Video fps.")
    ap.add_argument("--operations-to-frame", type=int, default=1000, help="Render one frame after this many parsed operations.")
    ap.add_argument("--sample-ops", type=int, default=None, help="Deprecated alias for --operations-to-frame.")
    ap.add_argument("--frame-every-ops", type=int, default=None, help="Alias for --operations-to-frame.")
    ap.add_argument("--frames", type=int, default=0, help="Stop after this many frames. 0 means no limit.")
    ap.add_argument("--title", default="", help="Title shown in the video.")
    ap.add_argument("--tmp-dir", default="", help="Working directory for mmap files and PNG frames. If set, files are kept there.")
    ap.add_argument("--keep-tmp", action="store_true", help="Keep temporary mmap files and PNG frames.")
    ap.add_argument("--clean-tmp", action="store_true", help="Delete old frames in --tmp-dir before rendering.")
    ap.add_argument("--no-video", action="store_true", help="Only write PNG frames, do not call ffmpeg.")
    ap.add_argument("--quiet-ffmpeg", action="store_true", help="Hide ffmpeg output.")
    ap.add_argument("--progress-every-seconds", type=float, default=2.0, help="Print progress every N seconds. 0 disables periodic progress.")
    args = ap.parse_args()

    if args.sample_ops is not None:
        args.operations_to_frame = args.sample_ops
        print("Warning: --sample-ops is deprecated. Use --operations-to-frame.", file=sys.stderr)
    if args.frame_every_ops is not None:
        args.operations_to_frame = args.frame_every_ops
        print("Warning: --frame-every-ops is an alias. Prefer --operations-to-frame.", file=sys.stderr)

    if args.operations_to_frame < 1:
        print("--operations-to-frame must be at least 1", file=sys.stderr)
        return 2

    Renderer(args).run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
