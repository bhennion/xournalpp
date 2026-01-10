#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
import tempfile
import types
from pathlib import Path
from typing import List


def parse_args(argv: List[str]) -> types.SimpleNamespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s",
        "--source",
        help="GTK install prefix to copy",
        type=Path,
        default=Path.home() / "gtk" / "inst",
    )
    parser.add_argument(
        "-o", "--output", help="Write tar file to this path", type=Path, required=True
    )
    parser.add_argument("--zstd", action="store_true", help="Use zstd compression")
    parser.add_argument(
        "--split",
        action="store_true",
        help="Split archive into 50 MB chunks (e.g., for CI)",
    )
    return parser.parse_args(argv)


def main(argv: List[str]):
    args = parse_args(argv[1:])
    # Validate arguments
    if not args.source.exists():
        exit(f"GTK install prefix '{args.source}' doesn't exist!")
    if not args.output.parent.exists():
        exit(f"Output directory '{args.output.parent}' doesn't exist!")

    zstd_bin = shutil.which("zstd")
    if args.zstd and zstd_bin is None:
        exit("--zstd specified, but zstd command not found")

    print("Packaging GTK installation for pipeline usage. This may take a while...")
    stage_tmpfile = tempfile.TemporaryDirectory()
    output_dir = Path(stage_tmpfile.name)
    stage_dir = output_dir / "gtk" / "inst"
    stage_dir.mkdir(parents=True)

    def copy2_custom(src, dst):
        """copy2 that does not follow symlinks"""
        return shutil.copy2(src, dst, follow_symlinks=False)

    def copytree_custom(*args, **kwargs):
        return shutil.copytree(
            *args, **kwargs, copy_function=copy2_custom, symlinks=True
        )

    def copy_to_stage(src_relpath: Path):
        srcp = args.source / src_relpath
        dstp = stage_dir / src_relpath
        assert srcp != dstp
        dstp.parent.mkdir(parents=True, exist_ok=True)
        copy2_custom(srcp, dstp)

    def copytree_to_stage(src_relpath: Path):
        srcp = args.source / src_relpath
        dstp = stage_dir / src_relpath
        assert srcp != dstp
        dstp.parent.mkdir(parents=True, exist_ok=True)
        copytree_custom(srcp, dstp)

    # Copy CMake
    # copy_to_stage(Path("bin") / "cmake")
    # cmake_data = list(args.source.glob("share/cmake-*"))
    # assert cmake_data, "share/cmake-* dir not found"
    # assert len(cmake_data) == 1, "more than one share/cmake-* dir unexpectedly found"
    # copytree_custom(cmake_data[0], stage_dir / "share" / cmake_data[0].name)

    # Copy CTest
    copy_to_stage(Path("bin") / "ctest")

    # Copy gtk3-demo
    copy_to_stage(Path("bin") / "gtk3-demo")

    # Copy pkgconf
    copy_to_stage(Path("bin") / "pkgconf")
    copytree_to_stage(Path("share") / "pkgconfig")

    # Copy gettext
    gettext_bins = [
        Path("bin") / "gettext",
        Path("bin") / "gettext.sh",
        Path("bin") / "xgettext",
        Path("bin") / "msgmerge",
        Path("bin") / "msgfmt",
        Path("bin") / "msgcat",
    ]
    for p in gettext_bins:
        copy_to_stage(p)
    for p in args.source.glob("share/gettext*"):
        copytree_custom(p, stage_dir / "share" / p.name)

    # Copy libraries
    copytree_to_stage("lib")
    copytree_to_stage("include")
    # Remove python3 as it takes a lot of space
    for p in (stage_dir / "lib").glob("python3.*"):
        shutil.rmtree(p)

    # Copy files required by bundler
    for p in [
        Path("glib-2.0") / "schemas",
        "locale",
        "themes",
        "icons",
        "gtksourceview-4",
        "lua",
        "poppler",
    ]:
        copytree_to_stage(Path("share") / p)
    copy_to_stage(Path("bin") / "gdk-pixbuf-query-loaders")
    copy_to_stage(Path("bin") / "gtk-query-immodules-3.0")

    subprocess.run(["find", stage_dir])

    output_arg: str
    if args.split:
        output_arg = "-"
        print(f"Generating split archive {args.output}.*")
    else:
        output_arg = args.output
        if args.output.exists():
            args.output.unlink()
        print(f"Generating archive {args.output}")

    compress_args = ["--use-compress-program", zstd_bin] if args.zstd else ["-z"]
    tar_cmd = ["tar", *compress_args, "-cf", output_arg, "-C", output_dir, "gtk"]

    if args.split:
        tar_proc = subprocess.Popen(tar_cmd, stdout=subprocess.PIPE)
        split_proc = subprocess.run(
            ["split", "-b", "50m", "-", str(args.output) + "."],
            stdin=tar_proc.stdout,
            check=True,
        )
        tar_proc.communicate()
        tar_proc.wait()
    else:
        subprocess.run(tar_cmd, check=True)


if __name__ == "__main__":
    main(sys.argv)
