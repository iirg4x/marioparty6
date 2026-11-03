"""Set the project wrapper's raw vendor boundary, then run the wrapped recipe."""
from pathlib import Path
import argparse
import os
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--vendor-root', default='include/PowerPC_EABI_Support/Msl')
parser.add_argument('command', nargs=argparse.REMAINDER)
args = parser.parse_args()
if not args.command:
    parser.error('expected sjiswrap executable and compiler recipe')
env = os.environ.copy()
env['SJISWRAP_EXCLUDE_DIRS'] = str(Path(args.vendor_root).resolve(strict=True))
raise SystemExit(subprocess.call(args.command, env=env))
