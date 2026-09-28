#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)
from tools.check_tu_declarations import check_policy_or_exit

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GP6E01",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--no-asm",
    action="store_true",
    help="don't incorporate .s files from asm directory",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.debug = args.debug
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
if not is_windows():
    config.wrapper = args.wrapper
if args.no_asm:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v0.9.2"
config.sjiswrap_tag = "v1.1.1"
config.wibo_tag = "0.6.11"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym version={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
    # "-listclosure", # Uncomment for Wii linkers
]
# Re-run the mdpartydll declaration-ownership gate whenever its contract or
# any governed source/header changes.  symbols.txt is deliberately omitted:
# DTK rewrites it during normal builds, which would create a configure loop.
tu_declarations_policy = Path("config") / config.version / "tu_declarations.json"
config.reconfig_deps = [
    Path("tools/check_tu_declarations.py"),
    tu_declarations_policy,
    Path("src/REL/mdpartydll/mdparty.c"),
    Path("src/REL/mdpartydll/stage.c"),
    Path("include/REL/mdpartyDll.h"),
    Path("include/REL/mdpartyDll_globals.h"),
    Path("include/game/audio.h"),
    Path("include/game/data.h"),
    Path("include/game/board/main.h"),
]

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
    f"-DVERSION={version_num}",
]

# Debug flags
if config.debug:
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

cflags_trk = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-common off",
    "-sdata 0",
    "-sdata2 0",
    "-inline auto,deferred",
    "-enum min",
    "-sdatathreshold 0",
]

# MusyX flags
cflags_musyx = [
    *cflags_base,
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_VERSION_MAJOR=2",
    "-DMUSY_VERSION_MINOR=0",
    "-DMUSY_VERSION_PATCH=4",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
    "-sdata 0",
    "-sdata2 0",
]

# Game flags
cflags_game = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
]

# Zlib flags
cflags_zlib = [
    *cflags_base,
    "-O0,p",
    "-fp_contract off",
]


# Game flags
cflags_libhu = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
]

# Game flags
cflags_msm = [
    *cflags_base,
]


# Mic SDK flags
cflags_dolphin = [
    *cflags_base,
]

cflags_thp = [
    *cflags_base,
]


# Mic SDK flags
cflags_sdk_mic = [
    *cflags_base,
]

# Game Speech SDK flags
cflags_gssdk = [
    *cflags_base,
    "-fp_contract off",
]

config.linker_version = "GC/2.6"
config.rel_strip_partial = False
config.rel_empty_file = "REL/empty.c"
config.rel_ldscript_replacements = {
    "w01Dll": [
        (
            "        .text ALIGN(0x4):{}",
            """        .text ALIGN(0x4):{
            world01.o(.text)
            world01.o(.text.common)
            world01.o(.text.after_common)
            *(.text)
        }""",
        ),
    ],
    "s01Dll": [
        (
            "        .text ALIGN(0x4):{}",
            """        .text ALIGN(0x4):{
            s01.o(.text)
            s01.o(.text.common)
            s01.o(.text.after_common)
            *(.text)
        }""",
        ),
    ],
    "openingDll": [
        (
            "        .text ALIGN(0x4):{}",
            """        .text ALIGN(0x4):{
            opening.o(.text)
            opening.o(.text.object_setup)
            opening.o(.text.after_setup)
            *(.text)
        }""",
        ),
    ],
}

config.rel_ldscript_replacements["mdminidll"] = [
        (
            "        .rodata ALIGN(0x8):{}",
            """        .rodata ALIGN(0x8):{
            scalar_approach.o(.rodata)
            auto_03_00000010_rodata.o(.rodata)
            scalar_approach.o(.rodata.opacity_one)
            auto_03_00000070_rodata.o(.rodata)
            scalar_approach.o(.rodata.opacity_zero)
            auto_03_00000078_rodata.o(.rodata)
            scalar_approach.o(.rodata.scalar_sine_1484)
            auto_03_00000084_rodata.o(.rodata)
            scalar_approach.o(.rodata.scalar_arc_1ed8)
            angle_wrap_94.o(.rodata)
            scalar_approach.o(.rodata.player_count_selection_b6ec)
            scalar_approach.o(.rodata.model_trajectory_update_5f6c)
            scalar_approach.o(.rodata.opacity_ten)
            auto_03_000000A4_rodata.o(.rodata)
            scalar_approach.o(.rodata.camera_target_17294)
            scalar_approach.o(.rodata.model_signed_bias)
            scalar_approach.o(.rodata.opacity_thirty)
            auto_03_000000CC_rodata.o(.rodata)
            scalar_approach.o(.rodata.opacity_inverse)
            auto_03_000000F4_rodata.o(.rodata)
            scalar_approach.o(.rodata.object_trajectory_4790)
            auto_03_0000012C_rodata.o(.rodata)
            scalar_approach.o(.rodata.model_scale_16c)
            scalar_approach.o(.rodata.opacity_bias)
            scalar_approach.o(.rodata.model_scale_178)
            auto_03_0000017C_rodata.o(.rodata)
            scalar_approach.o(.rodata.choice_math)
            scalar_approach.o(.rodata.model_selection_11e14)
            auto_03_00000238_rodata.o(.rodata)
            particle_sine_1f3ac.o(.rodata)
            particle_sine_1f3ac.o(.rodata.particle_zero_2f8)
            particle_sine_1f3ac.o(.rodata.particle_rays_2105c)
            particle_sine_1f3ac.o(.rodata.particle_origin_328)
            particle_sine_1f3ac.o(.rodata.particle_spiral_2185c)
            particle_sine_1f3ac.o(.rodata.particle_scatter_221e0)
            auto_03_00000378_rodata.o(.rodata)
            particle_sine_1f3ac.o(.rodata.particle_inverse)
            auto_03_0000038C_rodata.o(.rodata)
            particle_sine_1f3ac.o(.rodata.particle_palette_activation)
            particle_sine_1f3ac.o(.rodata.particle_palette_burst)
            particle_sine_1f3ac.o(.rodata.particle_palette_rows)
            auto_03_00000400_rodata.o(.rodata)
            runtime.o(.rodata)
        }""",
        ),
    ]

config.rel_pool_exports["mdminidll"] = [
        {'source': 'REL/mdminidll/object_release.c', 'text_members': [{'source': 'REL/mdminidll/secondary_object_release', 'section': '.text.secondary_object_release'}, {'source': 'REL/mdminidll/object_animation_pair_release', 'section': '.text.object_animation_pair_release'}, {'source': 'REL/mdminidll/scene_exit_15e0c', 'section': '.text.scene_exit'}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/scalar_sine_pool_80', 'symbol': 'lbl_1_rodata_80', 'section': '.rodata', 'native_section': '.rodata.scalar_sine_1484', 'native_offset': 0, 'linked_offset': 128, 'size': 4},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/scalar_arc_pool_90', 'symbol': 'lbl_1_rodata_90', 'section': '.rodata', 'native_section': '.rodata.scalar_arc_1ed8', 'native_offset': 0, 'linked_offset': 144, 'size': 4},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'text_members': [{'source': 'REL/mdminidll/particle_scalar_lerp.c', 'section': '.text.pool_1f494'}, {'source': 'REL/mdminidll/particle_scalar_approach.c', 'section': '.text.pool_1f4d8'}, {'source': 'REL/mdminidll/material_hook_1f574.c', 'section': '.text.pool_1f574'}, {'source': 'REL/mdminidll/layer_effect_tick.c', 'section': '.text.pool_1f9f4'}, {'source': 'REL/mdminidll/layer_effect_start.c', 'section': '.text.pool_1faf4'}, {'source': 'REL/mdminidll/layer_effect_create.c', 'section': '.text.pool_1fbe8'}, {'source': 'REL/mdminidll/particle_render_20068', 'section': '.text.pool_20068'}, {'source': 'REL/mdminidll/particle_fade_update_20b74', 'section': '.text.pool_20b74'}, {'source': 'REL/mdminidll/particle_setup_four_215a4', 'section': '.text.pool_215a4'}, {'source': 'REL/mdminidll/particle_update_23700', 'section': '.text.pool_23700'}, {'source': 'REL/mdminidll/particle_update_24524', 'section': '.text.pool_24524'}, {'source': 'REL/mdminidll/particle_setup_24d80.c', 'section': '.text.pool_24d80'}, {'source': 'REL/mdminidll/particle_activate_20a74', 'section': '.text.particle_activate'}, {'source': 'REL/mdminidll/particle_byte_update_22ebc', 'section': '.text.particle_byte_update'}, {'source': 'REL/mdminidll/hook_update_pair_23574', 'section': '.text.hook_update_pair'}, {'source': 'REL/mdminidll/hook_update_grid_2433c', 'section': '.text.hook_update_grid'}, {'source': 'REL/mdminidll/particle_controls', 'section': '.text.particle_controls'}, {'source': 'REL/mdminidll/particle_stop', 'section': '.text.particle_stop'}, {'source': 'REL/mdminidll/particle_pair_controls_23ec4', 'section': '.text.particle_pair_controls'}, {'source': 'REL/mdminidll/particle_color_259fc.c', 'section': '.text.particle_color'}, {'source': 'REL/mdminidll/particle_place_20f74.c', 'section': '.text.particle_place'}, {'source': 'REL/mdminidll/particle_palette_activate_25bf4', 'section': '.text.particle_palette_activation'}, {'source': 'REL/mdminidll/particle_position.c', 'section': '.text.particle_position'}, {'source': 'REL/mdminidll/particle_palette_burst_25ef0', 'section': '.text.particle_palette_burst'}, {'source': 'REL/mdminidll/particle_row_stop.c', 'section': '.text.particle_row_stop'}, {'source': 'REL/mdminidll/model_row_visibility.c', 'section': '.text.model_row_visibility'}, {'source': 'REL/mdminidll/particle_palette_rows_26454', 'section': '.text.particle_palette_rows'}, {'source': 'REL/mdminidll/particle_spiral_2185c', 'section': '.text.particle_spiral_2185c'}, {'source': 'REL/mdminidll/particle_scatter_221e0', 'section': '.text.particle_scatter_221e0'}, {'source': 'REL/mdminidll/particle_rays_2105c', 'section': '.text.particle_rays_2105c'}], 'pool_source': 'REL/mdminidll/particle_render_pool_288', 'symbol': 'lbl_1_rodata_288', 'section': '.rodata', 'native_offset': 0, 'linked_offset': 648, 'size': 112, 'members': [{'symbol': 'lbl_1_rodata_288', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_290', 'offset': 8, 'size': 8}, {'symbol': 'lbl_1_rodata_298', 'offset': 16, 'size': 4}, {'symbol': 'lbl_1_rodata_2A0', 'offset': 24, 'size': 8}, {'symbol': 'lbl_1_rodata_2A8', 'offset': 32, 'size': 4}, {'symbol': 'lbl_1_rodata_2AC', 'offset': 36, 'size': 4}, {'symbol': 'lbl_1_rodata_2B0', 'offset': 40, 'size': 4}, {'symbol': 'lbl_1_rodata_2B4', 'offset': 44, 'size': 4}, {'symbol': 'lbl_1_rodata_2B8', 'offset': 48, 'size': 4}, {'symbol': 'lbl_1_rodata_2BC', 'offset': 52, 'size': 4}, {'symbol': 'lbl_1_rodata_2C0', 'offset': 56, 'size': 4}, {'symbol': 'lbl_1_rodata_2C4', 'offset': 60, 'size': 4}, {'symbol': 'lbl_1_rodata_2C8', 'offset': 64, 'size': 4}, {'symbol': 'lbl_1_rodata_2CC', 'offset': 68, 'size': 4}, {'symbol': 'lbl_1_rodata_2D0', 'offset': 72, 'size': 4}, {'symbol': 'lbl_1_rodata_2D4', 'offset': 76, 'size': 4}, {'symbol': 'lbl_1_rodata_2D8', 'offset': 80, 'size': 4}, {'symbol': 'lbl_1_rodata_2DC', 'offset': 84, 'size': 4}, {'symbol': 'lbl_1_rodata_2E0', 'offset': 88, 'size': 4}, {'symbol': 'lbl_1_rodata_2E8', 'offset': 96, 'size': 8}, {'symbol': 'lbl_1_rodata_2F0', 'offset': 104, 'size': 8}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_palette_pool_3e4', 'symbol': 'lbl_1_rodata_3E4', 'section': '.rodata', 'native_section': '.rodata.particle_palette_rows', 'native_offset': 0, 'linked_offset': 996, 'size': 28, 'members': [{'symbol': 'lbl_1_rodata_3E4', 'offset': 0, 'size': 28}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_spiral_pool_32c', 'symbol': 'lbl_1_rodata_32C', 'section': '.rodata', 'native_section': '.rodata.particle_spiral_2185c', 'native_offset': 0, 'linked_offset': 812, 'size': 52, 'members': [{'symbol': 'lbl_1_rodata_32C', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_330', 'offset': 4, 'size': 4}, {'symbol': 'lbl_1_rodata_334', 'offset': 8, 'size': 4}, {'symbol': 'lbl_1_rodata_338', 'offset': 12, 'size': 4}, {'symbol': 'lbl_1_rodata_33C', 'offset': 16, 'size': 4}, {'symbol': 'lbl_1_rodata_340', 'offset': 20, 'size': 4}, {'symbol': 'lbl_1_rodata_344', 'offset': 24, 'size': 4}, {'symbol': 'lbl_1_rodata_348', 'offset': 28, 'size': 4}, {'symbol': 'lbl_1_rodata_34C', 'offset': 32, 'size': 4}, {'symbol': 'lbl_1_rodata_350', 'offset': 36, 'size': 4}, {'symbol': 'lbl_1_rodata_354', 'offset': 40, 'size': 4}, {'symbol': 'lbl_1_rodata_358', 'offset': 44, 'size': 4}, {'symbol': 'lbl_1_rodata_35C', 'offset': 48, 'size': 4}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_zero_2f8', 'symbol': 'lbl_1_rodata_2F8', 'section': '.rodata', 'native_section': '.rodata.particle_zero_2f8', 'native_offset': 0, 'linked_offset': 760, 'size': 8, 'members': [{'symbol': 'lbl_1_rodata_2F8', 'offset': 0, 'size': 8}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_origin_328', 'symbol': 'lbl_1_rodata_328', 'section': '.rodata', 'native_section': '.rodata.particle_origin_328', 'native_offset': 0, 'linked_offset': 808, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_328', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_scatter_360', 'symbol': 'lbl_1_rodata_360', 'section': '.rodata', 'native_section': '.rodata.particle_scatter_221e0', 'native_offset': 0, 'linked_offset': 864, 'size': 24, 'members': [{'symbol': 'lbl_1_rodata_360', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_364', 'offset': 4, 'size': 4}, {'symbol': 'lbl_1_rodata_368', 'offset': 8, 'size': 8}, {'symbol': 'lbl_1_rodata_370', 'offset': 16, 'size': 4}, {'symbol': 'lbl_1_rodata_374', 'offset': 20, 'size': 4}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_rays_pool_300', 'symbol': 'lbl_1_rodata_300', 'section': '.rodata', 'native_section': '.rodata.particle_rays_2105c', 'native_offset': 0, 'linked_offset': 768, 'size': 40, 'members': [{'symbol': 'lbl_1_rodata_300', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_304', 'offset': 4, 'size': 4}, {'symbol': 'lbl_1_rodata_308', 'offset': 8, 'size': 4}, {'symbol': 'lbl_1_rodata_30C', 'offset': 12, 'size': 4}, {'symbol': 'lbl_1_rodata_310', 'offset': 16, 'size': 4}, {'symbol': 'lbl_1_rodata_318', 'offset': 24, 'size': 8}, {'symbol': 'lbl_1_rodata_320', 'offset': 32, 'size': 8}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_inverse_pool_388', 'symbol': 'lbl_1_rodata_388', 'section': '.rodata', 'native_section': '.rodata.particle_inverse', 'native_offset': 0, 'linked_offset': 904, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_388', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_6c', 'symbol': 'lbl_1_rodata_6C', 'section': '.rodata', 'native_section': '.rodata.opacity_one', 'native_offset': 0, 'linked_offset': 108, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_6C', 'offset': 0, 'size': 4}], 'text_members': [{'source': 'REL/mdminidll/scalar_lerp.c', 'section': '.text.opacity_156c'}, {'source': 'REL/mdminidll/object_motion_reset.c', 'section': '.text.opacity_4e40'}, {'source': 'REL/mdminidll/camera_create.c', 'section': '.text.opacity_30a4'}, {'source': 'REL/mdminidll/object_turn_55b0.c', 'section': '.text.opacity_55b0'}, {'source': 'REL/mdminidll/model_opacity_begin_66f4', 'section': '.text.opacity_66f4'}, {'source': 'REL/mdminidll/model_opacity_7414', 'section': '.text.opacity_7414'}, {'source': 'REL/mdminidll/model_opacity_update_7754', 'section': '.text.opacity_7754'}, {'source': 'REL/mdminidll/model_animation_set', 'section': '.text.model_animation_set'}, {'source': 'REL/mdminidll/player_animation_reset_e0b0', 'section': '.text.player_animation_reset'}, {'source': 'REL/mdminidll/model_player_transition_105dc', 'section': '.text.model_player_transition'}, {'source': 'REL/mdminidll/model_player_exit_1406c', 'section': '.text.model_player_exit'}, {'source': 'REL/mdminidll/model_player_exit_reset_14484', 'section': '.text.model_player_exit_reset'}, {'source': 'REL/mdminidll/model_position_scale_a700', 'section': '.text.model_position_scale'}, {'source': 'REL/mdminidll/model_exit_transition_12e88', 'section': '.text.model_exit_transition'}, {'source': 'REL/mdminidll/sprite_projection_81fc', 'section': '.text.sprite_projection'}, {'source': 'REL/mdminidll/sprite_setup_7c4c', 'section': '.text.sprite_setup'}, {'source': 'REL/mdminidll/model_yaw_b078', 'section': '.text.model_yaw'}, {'source': 'REL/mdminidll/model_rotation_transition_11b0c', 'section': '.text.model_rotation_transition'}, {'source': 'REL/mdminidll/model_transition_109d4.c', 'section': '.text.model_transition_root'}, {'source': 'REL/mdminidll/model_transition_a330', 'section': '.text.model_transition_a330'}, {'source': 'REL/mdminidll/sprite_group_attr.c', 'section': '.text.sprite_group_attributes'}, {'source': 'REL/mdminidll/sprite_opacity_hide_6dc4', 'section': '.text.sprite_opacity_hide_6dc4'}, {'source': 'REL/mdminidll/window_visibility.c', 'section': '.text.window_visibility'}, {'source': 'REL/mdminidll/window_messages.c', 'section': '.text.window_messages'}, {'source': 'REL/mdminidll/window_select.c', 'section': '.text.window_select'}, {'source': 'REL/mdminidll/active_window_messages.c', 'section': '.text.active_window_messages'}, {'source': 'REL/mdminidll/active_window_choice.c', 'section': '.text.active_window_choice'}, {'source': 'REL/mdminidll/window_choice_181cc', 'section': '.text.window_choice_181cc'}, {'source': 'REL/mdminidll/direction_choice_97d8', 'section': '.text.direction_choice_97d8'}, {'source': 'REL/mdminidll/sprite_group_select_7280.c', 'section': '.text.sprite_group_select'}, {'source': 'REL/mdminidll/model_row_setup.c', 'section': '.text.model_row_setup'}, {'source': 'REL/mdminidll/model_selection_11e14', 'section': '.text.model_selection_11e14'}, {'source': 'REL/mdminidll/model_spin_shrink_d09c', 'section': '.text.model_spin_shrink_d09c'}, {'source': 'REL/mdminidll/vector_approach.c', 'section': '.text.vector_approach'}, {'source': 'REL/mdminidll/camera_approach_25b4', 'section': '.text.camera_25b4'}, {'source': 'REL/mdminidll/model_animation_turn_acf0', 'section': '.text.model_animation_turn_acf0'}, {'source': 'REL/mdminidll/model_player_select_c770', 'section': '.text.model_player_select_c770'}, {'source': 'REL/mdminidll/camera_target_17294', 'section': '.text.camera_target_17294'}, {'source': 'REL/mdminidll/model_config_enter_f6f8', 'section': '.text.model_config_enter_f6f8'}, {'source': 'REL/mdminidll/model_config_exit_fcc0', 'section': '.text.model_config_exit_fcc0'}, {'source': 'REL/mdminidll/model_mode_exit_124a0', 'section': '.text.model_mode_exit_124a0'}, {'source': 'REL/mdminidll/model_spin_grow_9f24', 'section': '.text.model_spin_grow_9f24'}, {'source': 'REL/mdminidll/sprite_row_reveal_7f8c.c', 'section': '.text.sprite_row_reveal'}, {'source': 'REL/mdminidll/model_mode_enter_148b4', 'section': '.text.model_mode_enter_148b4'}, {'source': 'REL/mdminidll/scalar_ease.c', 'section': '.text.scalar_ease'}, {'source': 'REL/mdminidll/model_trajectory_update_5f6c', 'section': '.text.model_trajectory_update_5f6c'}, {'source': 'REL/mdminidll/window_wait.c', 'section': '.text.mode_driver_34e4'}, {'source': 'REL/mdminidll/window_close_reset.c', 'section': '.text.mode_driver_3b10'}, {'source': 'REL/mdminidll/active_window_wait.c', 'section': '.text.mode_driver_3bcc'}, {'source': 'REL/mdminidll/object_motion_gate.c', 'section': '.text.mode_driver_5b48'}, {'source': 'REL/mdminidll/object_reset.c', 'section': '.text.mode_driver_7014'}, {'source': 'REL/mdminidll/sequence_run.c', 'section': '.text.mode_driver_b348'}, {'source': 'REL/mdminidll/window_transition_17ef4.c', 'section': '.text.mode_driver_17ef4'}, {'source': 'REL/mdminidll/window_transition_18650.c', 'section': '.text.mode_driver_18650'}, {'source': 'REL/mdminidll/mode_selection_run_1bcac.c', 'section': '.text.mode_driver_1bcac'}, {'source': 'REL/mdminidll/mode_driver_1e86c', 'section': '.text.mode_driver_1e86c'}, {'source': 'REL/mdminidll/model_trajectory_update_4eb4', 'section': '.text.model_trajectory_update_4eb4'}, {'source': 'REL/mdminidll/model_trajectory_update_1fac', 'section': '.text.model_trajectory_update_1fac'}, {'source': 'REL/mdminidll/mode_choice_1d640', 'section': '.text.mode_choice_1d640'}, {'source': 'REL/mdminidll/camera_debug_2878', 'section': '.text.camera_debug_2878'}, {'source': 'REL/mdminidll/scalar_sine_1484.c', 'section': '.text.scalar_sine_1484'}, {'source': 'REL/mdminidll/scalar_arc_1ed8.c', 'section': '.text.scalar_arc_1ed8'}, {'source': 'REL/mdminidll/object_trajectory_4790', 'section': '.text.object_trajectory_4790'}, {'source': 'REL/mdminidll/scalar_row_reset.c', 'section': '.text.scalar_row_reset_8188'}, {'source': 'REL/mdminidll/position_offset.c', 'section': '.text.position_offset_8914'}, {'source': 'REL/mdminidll/model_sprite_reset.c', 'section': '.text.model_sprite_reset_8b70'}, {'source': 'REL/mdminidll/model_sprite_layout_899c.c', 'section': '.text.model_sprite_layout_899c'}, {'source': 'REL/mdminidll/model_rotation.c', 'section': '.text.model_rotation_d450'}, {'source': 'REL/mdminidll/player_selection_d554', 'section': '.text.player_selection_d554'}, {'source': 'REL/mdminidll/sprite_project_position.c', 'section': '.text.sprite_project_position'}, {'source': 'REL/mdminidll/sprite_bank.c', 'section': '.text.sprite_bank_81b0'}, {'source': 'REL/mdminidll/computer_player_selection_e1c0', 'section': '.text.computer_player_selection_e1c0'}, {'source': 'REL/mdminidll/player_count_selection_b6ec', 'section': '.text.player_count_selection_b6ec'}, {'source': 'REL/mdminidll/model_mode_exit_1325c', 'section': '.text.model_mode_exit_1325c'}, {'source': 'REL/mdminidll/player_config.c', 'section': '.text.mode_player_config'}, {'source': 'REL/mdminidll/secondary_window_message.c', 'section': '.text.mode_secondary_window'}, {'source': 'REL/mdminidll/sequence_done.c', 'section': '.text.mode_ready_b310'}, {'source': 'REL/mdminidll/player_selection.c', 'section': '.text.mode_player_selection'}, {'source': 'REL/mdminidll/mode_selection_18a5c', 'section': '.text.mode_selection_18a5c'}, {'source': 'REL/mdminidll/mode_selection_1a6c0', 'section': '.text.mode_selection_1a6c0'}, {'source': 'REL/mdminidll/mode_selection_1bf60', 'section': '.text.mode_selection_1bf60'}, {'source': 'REL/mdminidll/mode_selection_19940', 'section': '.text.mode_selection_19940'}, {'source': 'REL/mdminidll/window_transition_184d4', 'section': '.text.mode_driver_184d4'}, {'source': 'REL/mdminidll/mode_exit_1e1ac', 'section': '.text.mode_exit_1e1ac'}, {'source': 'REL/mdminidll/camera_callback.c', 'section': '.text.camera_callback_2868'}, {'source': 'REL/mdminidll/object_trajectory_5434.c', 'section': '.text.object_trajectory_5434'}, {'source': 'REL/mdminidll/object_trajectory_start_6520.c', 'section': '.text.object_trajectory_start_6520'}, {'source': 'REL/mdminidll/player_config_apply_c90.c', 'section': '.text.player_config_apply_c90'}, {'source': 'REL/mdminidll/resources.c', 'section': '.text.resources_a4c'}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/trajectory_weight_pool_9c', 'symbol': 'lbl_1_rodata_9C', 'section': '.rodata', 'native_section': '.rodata.model_trajectory_update_5f6c', 'native_offset': 0, 'linked_offset': 156, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_9C', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/negative_angle_pool_98', 'symbol': 'lbl_1_rodata_98', 'section': '.rodata', 'native_section': '.rodata.player_count_selection_b6ec', 'native_offset': 0, 'linked_offset': 152, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_98', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/trajectory_duration_pool_128', 'symbol': 'lbl_1_rodata_128', 'section': '.rodata', 'native_section': '.rodata.object_trajectory_4790', 'native_offset': 0, 'linked_offset': 296, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_128', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/camera_target_pool_b8', 'symbol': 'lbl_1_rodata_B8', 'section': '.rodata', 'native_section': '.rodata.camera_target_17294', 'native_offset': 0, 'linked_offset': 184, 'size': 8, 'members': [{'symbol': 'lbl_1_rodata_B8', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_BC', 'offset': 4, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/model_scale_pool_16c', 'symbol': 'lbl_1_rodata_16C', 'section': '.rodata', 'native_section': '.rodata.model_scale_16c', 'native_offset': 0, 'linked_offset': 364, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_16C', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/model_scale_pool_178', 'symbol': 'lbl_1_rodata_178', 'section': '.rodata', 'native_section': '.rodata.model_scale_178', 'native_offset': 0, 'linked_offset': 376, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_178', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/model_selection_pool_230', 'symbol': 'lbl_1_rodata_230', 'section': '.rodata', 'native_section': '.rodata.model_selection_11e14', 'native_offset': 0, 'linked_offset': 560, 'size': 8, 'members': [{'symbol': 'lbl_1_rodata_230', 'offset': 0, 'size': 4}, {'symbol': 'lbl_1_rodata_234', 'offset': 4, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/direction_math_pool_220', 'symbol': 'lbl_1_rodata_220', 'section': '.rodata', 'native_section': '.rodata.choice_math', 'native_offset': 0, 'linked_offset': 544, 'size': 16, 'members': [{'symbol': 'lbl_1_rodata_220', 'offset': 0, 'size': 8}, {'symbol': 'lbl_1_rodata_228', 'offset': 8, 'size': 8}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_74', 'symbol': 'lbl_1_rodata_74', 'section': '.rodata', 'native_section': '.rodata.opacity_zero', 'native_offset': 0, 'linked_offset': 116, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_74', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_a0', 'symbol': 'lbl_1_rodata_A0', 'section': '.rodata', 'native_section': '.rodata.opacity_ten', 'native_offset': 0, 'linked_offset': 160, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_A0', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_c8', 'symbol': 'lbl_1_rodata_C8', 'section': '.rodata', 'native_section': '.rodata.opacity_thirty', 'native_offset': 0, 'linked_offset': 200, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_C8', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_f0', 'symbol': 'lbl_1_rodata_F0', 'section': '.rodata', 'native_section': '.rodata.opacity_inverse', 'native_offset': 0, 'linked_offset': 240, 'size': 4, 'members': [{'symbol': 'lbl_1_rodata_F0', 'offset': 0, 'size': 4}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'pool_source': 'REL/mdminidll/opacity_pool_170', 'symbol': 'lbl_1_rodata_170', 'section': '.rodata', 'native_section': '.rodata.opacity_bias', 'native_offset': 0, 'linked_offset': 368, 'size': 8, 'members': [{'symbol': 'lbl_1_rodata_170', 'offset': 0, 'size': 8}]},
        {'source': 'REL/mdminidll/scalar_approach.c', 'symbol': 'lbl_1_rodata_C0', 'section': '.rodata', 'native_offset': 0, 'linked_offset': 192, 'size': 8, 'pool_source': 'REL/mdminidll/scalar_model_pool_c0', 'native_section': '.rodata.model_signed_bias'},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_palette_pool_3b0', 'symbol': 'lbl_1_rodata_3B0', 'section': '.rodata', 'native_section': '.rodata.particle_palette_activation', 'native_offset': 0, 'linked_offset': 944, 'size': 20, 'members': [{'symbol': 'lbl_1_rodata_3B0', 'offset': 0, 'size': 20}]},
        {'source': 'REL/mdminidll/particle_sine_1f3ac.c', 'pool_source': 'REL/mdminidll/particle_palette_pool_3c4', 'symbol': 'lbl_1_rodata_3C4', 'section': '.rodata', 'native_section': '.rodata.particle_palette_burst', 'native_offset': 0, 'linked_offset': 964, 'size': 32, 'members': [{'symbol': 'lbl_1_rodata_3C4', 'offset': 0, 'size': 20}, {'symbol': 'lbl_1_rodata_3D8', 'offset': 20, 'size': 4}, {'symbol': 'lbl_1_rodata_3DC', 'offset': 24, 'size': 4}, {'symbol': 'lbl_1_rodata_3E0', 'offset': 28, 'size': 4}]},
    ]

# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_dolphin,
        "host": False,
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "host": True,
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching

config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(Matching, "Runtime.PPCEABI.H/__va_arg.c"),
            Object(Matching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(
                Matching,
                "Runtime.PPCEABI.H/New.cp",
                extra_cflags=["-Cpp_exceptions on", "-inline auto,deferred"],
            ),
            Object(
                Matching,
                "Runtime.PPCEABI.H/NewMore.cp",
                extra_cflags=[
                    "-Cpp_exceptions on",
                    "-RTTI on",
                    "-inline auto,deferred",
                    "-str nopool",
                ],
            ),
            Object(
                Matching,
                "Runtime.PPCEABI.H/NMWException.cpp",
                mw_version="GC/1.3.2",
                extra_cflags=["-Cpp_exceptions on", "-inline auto,deferred"],
            ),
            Object(Matching, "Runtime.PPCEABI.H/ptmf.c"),
            Object(Matching, "Runtime.PPCEABI.H/runtime.c"),
            Object(Matching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
            Object(
                NonMatching,
                "Runtime.PPCEABI.H/Gecko_ExceptionPPC.cpp",
                cflags=[
                    (
                        "-str reuse,readonly"
                        if flag.startswith("-str ")
                        else "-inline auto,deferred"
                        if flag.startswith("-inline ")
                        else flag
                    )
                    for flag in cflags_runtime
                ],
                extra_cflags=["-Cpp_exceptions on", "-RTTI on"],
            ),
            Object(
                Matching,
                "Runtime.PPCEABI.H/GCN_mem_alloc.c",
                cflags=[
                    "-str reuse,readonly" if flag.startswith("-str ") else flag
                    for flag in cflags_runtime
                ],
            ),
            Object(Matching, "Runtime.PPCEABI.H/__mem.c"),
        ],
    },
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": config.linker_version,
        "cflags": [*cflags_runtime, "-inline deferred"],
        "host": False,
        "objects": [
            Object(Matching, "MSL_C.PPCEABI.bare.H/abort_exit.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/alloc.c", mw_version="GC/1.3"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/errno.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ansi_files.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/ansi_fp.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/arith.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/assert.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/buffer_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ctype.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/direct_io.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/file_io.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/FILE_POS.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/mbstring.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mem.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/mem_funcs.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/misc_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/printf.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/qsort.c",
                extra_cflags=["-opt nopropagation"],
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/float.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/signal.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/string.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/uart_console_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/wchar_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_acos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_asin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_exp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_log.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_log10.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/e_pow.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_sin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_tan.c"),
            Object(
                Matching,
                "MSL_C.PPCEABI.bare.H/s_atan.c",
                mw_version="GC/1.3",
            ),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_ceil.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_copysign.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_floor.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_frexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_ldexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_modf.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_sin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_tan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_acos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_asin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_exp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_log.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_log10.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_pow.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/math_ppc.c"),
        ],
    },
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": "GC/1.3",
        "cflags": cflags_trk,
        "host": False,
        "objects": [
            Object(Matching, "TRK_MINNOW_DOLPHIN/mainloop.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msg.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/serpoll.c"),
            Object(
                Matching,
                "TRK_MINNOW_DOLPHIN/usr_put.c",
                mw_version="GC/1.3.2",
            ),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dispatch.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/support.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/notify.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targsupp.s"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/__exception.s"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mslsupp.c"),
        ],
    },
    {
        "lib": "musyx",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_musyx,
        "host": False,
        "objects": [
            Object(Matching, "musyx/runtime/seq.c"),
            Object(Matching, "musyx/runtime/synth.c"),
            Object(Matching, "musyx/runtime/seq_api.c"),
            Object(Matching, "musyx/runtime/snd_synthapi.c"),
            Object(Matching, "musyx/runtime/stream.c"),
            Object(Matching, "musyx/runtime/synthdata.c"),
            Object(Matching, "musyx/runtime/synthmacros.c"),
            Object(Matching, "musyx/runtime/synthvoice.c"),
            Object(Matching, "musyx/runtime/synth_ac.c"),
            Object(Matching, "musyx/runtime/synth_dbtab.c"),
            Object(Matching, "musyx/runtime/synth_adsr.c"),
            Object(Matching, "musyx/runtime/synth_vsamples.c"),
            Object(Matching, "musyx/runtime/s_data.c"),
            Object(Matching, "musyx/runtime/hw_dspctrl.c"),
            Object(Matching, "musyx/runtime/hw_volconv.c"),
            Object(Matching, "musyx/runtime/snd3d.c"),
            Object(Matching, "musyx/runtime/snd_init.c"),
            Object(Matching, "musyx/runtime/snd_math.c"),
            Object(Matching, "musyx/runtime/snd_midictrl.c"),
            Object(Matching, "musyx/runtime/snd_service.c"),
            Object(Matching, "musyx/runtime/hardware.c"),
            Object(Matching, "musyx/runtime/dsp_import.c"),
            Object(Matching, "musyx/runtime/hw_aramdma.c"),
            Object(Matching, "musyx/runtime/hw_dolphin.c"),
            Object(Matching, "musyx/runtime/hw_memory.c"),
            Object(Matching, "musyx/runtime/CheapReverb/creverb_fx.c"),
            Object(Matching, "musyx/runtime/CheapReverb/creverb.c"),
            Object(Matching, "musyx/runtime/StdReverb/reverb_fx.c"),
            Object(Matching, "musyx/runtime/StdReverb/reverb.c"),
            Object(Matching, "musyx/runtime/Delay/delay_fx.c"),
            Object(Matching, "musyx/runtime/Chorus/chorus_fx.c"),
        ],
    },
    {
        "lib": "OdemuExi2",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(
                Matching,
                "OdemuExi2/DebuggerDriver.c",
                cflags=[
                    "-nodefaults",
                    "-proc gekko",
                    "-align powerpc",
                    "-enum int",
                    "-multibyte",
                    "-char unsigned",
                    "-fp hardware",
                    "-Cpp_exceptions off",
                    '-pragma "cats off"',
                    '-pragma "warn_notinlined off"',
                    "-maxerrors 1",
                    "-nosyspath",
                    "-i include",
                    f"-i build/{config.version}/include",
                    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
                    f"-DVERSION={version_num}",
                    "-DNDEBUG=1",
                    "-O4,p",
                    "-inline all",
                    "-char signed",
                ],
                extra_cflags=["-inline deferred"],
                mw_version="GC/1.2.5n",
            ),
        ],
    },
    {
        "lib": "amcstubs",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(Matching, "amcstubs/AmcExi2Stubs.c"),
        ],
    },
    {
        "lib": "odenotstub",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(Matching, "odenotstub/odenotstub.c"),
        ],
    },
    {
        "lib": "Game",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "host": False,
        "objects": [
            Object(Matching, "game/main.c"),
            Object(Matching, "game/pad.c"),
            Object(Matching, "game/dvd.c"),
            Object(Matching, "game/data.c"),
            Object(Matching, "game/decode.c"),
            Object(Matching, "game/font.c"),
            Object(Matching, "game/init.c"),
            Object(Matching, "game/jmp.c"),
            Object(Matching, "game/malloc.c"),
            Object(Matching, "game/memory.c"),
            Object(Matching, "game/printfunc.c"),
            Object(Matching, "game/process.c"),
            Object(Matching, "game/sprman.c"),
            Object(Matching, "game/sprput.c"),
            Object(Matching, "game/hsfload.c"),
            Object(Matching, "game/hsfdraw.c"),
            Object(Matching, "game/hsfman.c"),
            Object(Matching, "game/hsfmotion.c"),
            Object(Matching, "game/hsfanim.c"),
            Object(Matching, "game/hsfex.c"),
            Object(Matching, "game/perf.c"),
            Object(Matching, "game/objmain.c"),
            Object(Matching, "game/fault.c"),
            Object(Matching, "game/gamework.c"),
            Object(Matching, "game/objsysobj.c"),
            Object(Matching, "game/objdll.c"),
            Object(Matching, "game/frand.c"),
            Object(Matching, "game/audio.c"),
            Object(Matching, "game/EnvelopeExec.c"),
            Object(Matching, "game/gamemes.c"),
            Object(Matching, "game/esprite.c"),
            Object(Matching, "game/ovllist.c"),
            Object(Matching, "game/ClusterExec.c"),
            Object(Matching, "game/ShapeExec.c"),
            Object(Matching, "game/wipe.c"),
            Object(Matching, "game/window.c"),
            Object(Matching, "game/card.c"),
            Object(Matching, "game/armem.c"),
            Object(Matching, "game/charman.c"),
            Object(Matching, "game/mapspace.c"),
            Object(Matching, "game/THPSimple.c"),
            Object(Matching, "game/THPDraw.c"),
            Object(Matching, "game/thpmain.c"),
            Object(Matching, "game/mgdata.c"),
            Object(Matching, "game/objsub.c"),
            Object(Matching, "game/flag.c"),
            Object(Matching, "game/saveload.c"),
            Object(Matching, "game/sreset.c"),
            Object(Matching, "game/mgtimer.c"),
            Object(Matching, "game/mgscore.c"),
            Object(Matching, "game/seqman.c"),
            Object(Matching, "game/colman.c"),
            Object(Matching, "game/actman.c"),
            Object(Matching, "game/mggamemes.c"),
            Object(Matching, "game/mic.c"),
            Object(Matching, "game/code_80146BA0.c"),
            Object(Matching, "game/kerent.c"),
            Object(Matching, "board/malloc.c"),
            Object(Matching, "board/comchoice.c"),
            Object(Matching, "board/board.c"),
            Object(Matching, "board/exit.c"),
        ],
    },
    DolphinLib(
        "base",
        [
            Object(Matching, "dolphin/base/PPCArch.c"),
        ],
    ),
    DolphinLib(
        "os",
        [
            Object(
                Matching,
                "dolphin/os/OS.c",
                extra_cflags=["-char unsigned", "-DSDK_REVISION=1"],
            ),
            Object(Matching, "dolphin/os/OSAlarm.c"),
            Object(Matching, "dolphin/os/OSAlloc.c"),
            Object(Matching, "dolphin/os/OSArena.c"),
            Object(Matching, "dolphin/os/OSAudioSystem.c"),
            Object(Matching, "dolphin/os/OSCache.c"),
            Object(Matching, "dolphin/os/OSContext.c"),
            Object(Matching, "dolphin/os/OSError.c"),
            Object(Matching, "dolphin/os/OSExec.c"),
            Object(Matching, "dolphin/os/OSFont.c", extra_cflags=["-char unsigned"]),
            Object(Matching, "dolphin/os/OSInterrupt.c"),
            Object(Matching, "dolphin/os/OSLink.c"),
            Object(Matching, "dolphin/os/OSMessage.c"),
            Object(Matching, "dolphin/os/OSMemory.c"),
            Object(Matching, "dolphin/os/OSMutex.c"),
            Object(Matching, "dolphin/os/OSReboot.c"),
            Object(Matching, "dolphin/os/OSReset.c"),
            Object(Matching, "dolphin/os/OSResetSW.c"),
            Object(Matching, "dolphin/os/OSRtc.c"),
            Object(Matching, "dolphin/os/OSSemaphore.c"),
            Object(Matching, "dolphin/os/OSStopwatch.c"),
            Object(Matching, "dolphin/os/OSSync.c"),
            Object(Matching, "dolphin/os/OSThread.c"),
            Object(Matching, "dolphin/os/OSTime.c"),
            Object(Matching, "dolphin/os/__start.c"),
            Object(Matching, "dolphin/os/__ppc_eabi_init.c"),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(Matching, "dolphin/db/db.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(Matching, "dolphin/mtx/mtx.c"),
            Object(Matching, "dolphin/mtx/mtxvec.c"),
            Object(Matching, "dolphin/mtx/mtx44.c"),
            Object(Matching, "dolphin/mtx/mtx44vec.c"),
            Object(Matching, "dolphin/mtx/vec.c"),
            Object(Matching, "dolphin/mtx/quat.c"),
            Object(Matching, "dolphin/mtx/psmtx.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(Matching, "dolphin/dvd/dvdlow.c"),
            Object(Matching, "dolphin/dvd/dvdfs.c"),
            Object(Matching, "dolphin/dvd/dvd.c"),
            Object(Matching, "dolphin/dvd/dvdqueue.c"),
            Object(Matching, "dolphin/dvd/dvderror.c"),
            Object(Matching, "dolphin/dvd/dvdidutils.c"),
            Object(Matching, "dolphin/dvd/dvdFatal.c"),
            Object(Matching, "dolphin/dvd/fstload.c"),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(Matching, "dolphin/vi/vi.c"),
        ],
    ),
    DolphinLib(
        "demo",
        [
            Object(Matching, "dolphin/demo/DEMOInit.c"),
            Object(Matching, "dolphin/demo/DEMOFont.c"),
            Object(Matching, "dolphin/demo/DEMOPuts.c"),
            Object(Matching, "dolphin/demo/DEMOStats.c"),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(
                Matching,
                "dolphin/pad/Padclamp.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(Matching, "dolphin/pad/Pad.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(Matching, "dolphin/ai/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(Matching, "dolphin/ar/ar.c"),
            Object(Matching, "dolphin/ar/arq.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(Matching, "dolphin/dsp/dsp.c"),
            Object(Matching, "dolphin/dsp/dsp_debug.c"),
            Object(Matching, "dolphin/dsp/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(
                Matching,
                "dolphin/gx/GXInit.c",
                extra_cflags=["-opt nopeephole"],
            ),
            Object(Matching, "dolphin/gx/GXFifo.c"),
            Object(Matching, "dolphin/gx/GXAttr.c"),
            Object(Matching, "dolphin/gx/GXMisc.c"),
            Object(Matching, "dolphin/gx/GXGeometry.c"),
            Object(Matching, "dolphin/gx/GXFrameBuf.c"),
            Object(Matching, "dolphin/gx/GXLight.c", extra_cflags=["-fp_contract off"]),
            Object(Matching, "dolphin/gx/GXTexture.c"),
            Object(Matching, "dolphin/gx/GXBump.c"),
            Object(Matching, "dolphin/gx/GXTev.c"),
            Object(Matching, "dolphin/gx/GXPixel.c"),
            Object(Matching, "dolphin/gx/GXDraw.c"),
            Object(Matching, "dolphin/gx/GXDisplayList.c"),
            Object(
                Matching,
                "dolphin/gx/GXTransform.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(Matching, "dolphin/gx/GXPerf.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(Matching, "dolphin/card/CARDBios.c"),
            Object(Matching, "dolphin/card/CARDUnlock.c"),
            Object(Matching, "dolphin/card/CARDRdwr.c"),
            Object(Matching, "dolphin/card/CARDBlock.c"),
            Object(Matching, "dolphin/card/CARDDir.c"),
            Object(Matching, "dolphin/card/CARDCheck.c"),
            Object(Matching, "dolphin/card/CARDMount.c"),
            Object(Matching, "dolphin/card/CARDFormat.c"),
            Object(Matching, "dolphin/card/CARDOpen.c"),
            Object(Matching, "dolphin/card/CARDCreate.c"),
            Object(Matching, "dolphin/card/CARDRead.c"),
            Object(Matching, "dolphin/card/CARDWrite.c"),
            Object(Matching, "dolphin/card/CARDDelete.c"),
            Object(Matching, "dolphin/card/CARDStat.c"),
            Object(Matching, "dolphin/card/CARDNet.c"),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(Matching, "dolphin/exi/EXIBios.c"),
            Object(Matching, "dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(Matching, "dolphin/si/SIBios.c"),
            Object(Matching, "dolphin/si/SISamplingRate.c"),
        ],
    ),
    {
        "lib": "thp",
        "mw_version": "GC/1.2.5",
        "cflags": cflags_thp,
        "host": False,
        "objects": [
            Object(Matching, "dolphin/thp/THPDec.c"),
            Object(Matching, "dolphin/thp/THPAudio.c"),
        ],
    },
    {
        "lib": "sdk_mic",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_sdk_mic,
        "host": False,
        "objects": [
            Object(
                NonMatching,
                "dolphin/mic/mic.c",
                extra_cflags=["-use_lmw_stmw on"],
            ),
            Object(
                Matching,
                "dolphin/mic/m2s.c",
                extra_cflags=["-use_lmw_stmw on"],
            ),
        ],
    },
    {
        "lib": "gssdk",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_gssdk,
        "host": False,
        "objects": [
            Object(Matching, "gssdk_lib/gsapi/sid/sid.c"),
            Object(NonMatching, "gssdk_lib/gsapi/callbacks.c"),
            Object(Matching, "gssdk_lib/gsapi/ctxfuncs.c"),
            Object(Matching, "gssdk_lib/gsapi/extaudio.c"),
            Object(NonMatching, "gssdk_lib/gsapi/gsapi.c"),
            Object(Matching, "gssdk_lib/gsapi/mathusage.c"),
            Object(Matching, "gssdk_lib/gsapi/wrddata.c"),
            Object(NonMatching, "gssdk_lib/asrpho/asrspi.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/convert.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/creasp.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/creaspch.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/creaspt.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/creatree.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/crsptrch.c"),
            Object(Matching, "gssdk_lib/asrpho/rec1600/ctrl.c"),
            Object(Matching, "gssdk_lib/asrpho/rec1600/initial.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/spi1600.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/train.c"),
            Object(NonMatching, "gssdk_lib/asrpho/rec1600/userword.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/delaybl.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/dpgenuw.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/dpscruw.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/exev_dp.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/fft_maye.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/fftmod.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/isoword.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/nbestdp.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/pitchdp.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/pitchwin.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/stacker.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/undersam.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/acne.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/dctlift.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/gender.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/logexp.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/mel.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/mtx.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/mtxopt.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/smoother.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/spline.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/specsub.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flblocks/vad.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/vq1500.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flblocks/window.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/combiner.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/dist16.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/genfilt.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flfxblks/lkahead.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flfxblks/median.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/pitchco.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/shs_vuv.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/slidhist.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flfxblks/statio.c"),
            Object(Matching, "gssdk_lib/asrpho/common/blocks/flfxblks/subsamp.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/trigglr.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/blocks/flfxblks/voicing.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/ctxdata/ctxdata.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/ctxdata/langdata.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/tos/mqueue.c"),
            Object(NonMatching, "gssdk_lib/asrpho/common/tos/tinyos.c"),
            Object(Matching, "gssdk_lib/asrpho/common/fastallo/fastallo.c"),
            Object(Matching, "gssdk_lib/common/csspi/csspi.c"),
            Object(Matching, "gssdk_lib/common/safeh/safeh.c"),
            Object(Matching, "gssdk_lib/common/osspi/osspi.c"),
            Object(Matching, "gssdk_lib/common/rsrc/rsrc.c"),
        ],
    },
    {
        "lib": "libhu",
        "mw_version": config.linker_version,
        "cflags": cflags_libhu,
        "host": False,
        "objects": [
            Object(Matching, "libhu/setvf.c"),
            Object(Matching, "libhu/subvf.c"),
        ],
    },
    {
        "lib": "msm",
        "mw_version": "GC/1.2.5",
        "cflags": cflags_msm,
        "host": False,
        "objects": [
            Object(NonMatching, "msm/msmsys.c"),
            Object(Matching, "msm/msmmem.c"),
            Object(Matching, "msm/msmfio.c"),
            Object(Matching, "msm/msmmus.c"),
            Object(Matching, "msm/msmse.c"),
            Object(NonMatching, "msm/msmstream.c"),
        ],
    },
    {
        "lib": "zlib",
        "mw_version": config.linker_version,
        "cflags": cflags_zlib,
        "host": False,
        "objects": [
            Object(Matching, "zlib/adler32.c"),
            Object(Matching, "zlib/inflate.c"),
            Object(Matching, "zlib/infblock.c"),
            Object(Matching, "zlib/infcodes.c"),
            Object(Matching, "zlib/infutil.c"),
            Object(Matching, "zlib/inftrees.c"),
            Object(Matching, "zlib/inffast.c"),
            Object(Matching, "zlib/zutil.c"),
        ],
    },
    {
        "lib": "board",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "host": False,
        "objects": [
            Object(Matching, "board/math.c", extra_cflags=["-O4,p", "-schedule off", "-opt nopeephole", "-char signed"]),
            Object(Matching, "board/camera.c"),
            Object(Matching, "board/player.c"),
            Object(Matching, "board/snpc.c"),
            Object(Matching, "board/object.c"),
            Object(Matching, "board/window.c"),
            Object(Matching, "board/audio.c"),
            Object(Matching, "board/scroll.c"),
            Object(Matching, "board/masu.c"),
            Object(Matching, "board/coin.c"),
            Object(Matching, "board/star.c"),
            Object(Matching, "board/padall.c"),
            Object(
                Matching,
                "board/dice.c",
                extra_cflags=["-O4,p", "-schedule off", "-opt nopeephole"],
            ),
            Object(Matching, "board/status.c"),
            Object(Matching, "board/opening.c"),
            Object(Matching, "board/pause.c"),
            Object(Matching, "board/tutorial.c"),
            Object(Matching, "board/roulette.c"),
            Object(Matching, "board/capselect.c"),
            Object(Matching, "board/capmove.c"),
            Object(Matching, "board/capthrow.c"),
            Object(Matching, "board/captrap.c"),
            Object(Matching, "board/capspecial.c"),
            Object(Matching, "board/capsule.c"),
            Object(Matching, "board/capevent.c"),
            Object(Matching, "board/shopevent.c"),
            Object(Matching, "board/guide.c"),
            Object(Matching, "board/branch.c"),
            Object(Matching, "board/mgcall.c"),
            Object(Matching, "board/effect.c", extra_cflags=["-O4,p", "-schedule off", "-opt nopeephole"]),
            Object(Matching, "board/config.c"),
            Object(Matching, "board/gate.c"),
            Object(Matching, "board/last5.c"),
            Object(Matching, "board/telop.c"),
            Object(Matching, "board/wipe.c"),
            Object(Matching, "board/single.c"),
        ],
    },
    {
        "lib": "REL",
        "mw_version": config.linker_version,
        "cflags": cflags_rel,
        "host": False,
        "objects": [
            Object(Matching, "REL/empty.c"),  # Must be marked as matching
            Object(
                Matching,
                "REL/runtime.c",
                source="REL/runtime.c",
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        ],
    },
    Rel(
        "bootDll",
        objects={
            Object(Matching, "REL/bootDll/boot.c", mw_version=config.linker_version),
            Object(Matching, "REL/bootDll/data.c"),
            Object(
                Matching,
                "REL/bootDll/opening.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off"],
            ),
        },
    ),
    Rel(
        "selmenuDll",
        objects={
            Object(
                Matching,
                "REL/selmenuDll/selmenu.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off"],
            ),
            Object(
                Matching,
                "REL/selmenuDll/runtime.c",
                source="REL/selmenuDll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-DMP6_SELMENU_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "fileseldll",
        objects={
            Object(Matching, "REL/fileseldll/filesel.c", mw_version=config.linker_version),
            Object(
                Matching,
                "REL/fileseldll/filename.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off"],
            ),
            Object(
                Matching,
                "REL/fileseldll/saveload.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off", "-inline noauto"],
            ),
            Object(
                Matching,
                "REL/fileseldll/runtime.c",
                source="REL/fileseldll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-DMP6_FILESEL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "mdseldll",
        objects={
            Object(Matching, "REL/mdseldll/mdsel.c"),
            Object(
                Matching,
                "REL/mdseldll/runtime.c",
                source="REL/mdseldll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-DMP6_MDSEL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "mdpartydll",
        objects={
            Object(Matching, "REL/mdpartydll/mdparty.c"),
            Object(
                Matching,
                "REL/mdpartydll/stage.c",
                extra_cflags=["-pooldata off"],
            ),
            Object(
                Matching,
                "REL/mdpartydll/runtime.c",
                source="REL/mdpartydll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "w01Dll",
        objects={
            Object(
                Matching,
                "REL/w01Dll/world01.c",
                mw_version="GC/2.7",
            ),
            Object(
                Matching,
                "REL/w01Dll/runtime.c",
                source="REL/w01Dll/runtime.c",
                mw_version="GC/2.7",
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "sequencedll",
        objects={
            Object(
                Matching,
                "REL/sequencedll/sequence.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off"],
            ),
        },
    ),
    Rel(
        "actmanDLL",
        objects={
            Object(Matching, "REL/actmanDLL/actman.c", mw_version=config.linker_version),
        },
    ),
    Rel(
        "meschkdll",
        objects={
            Object(
                Matching,
                "REL/meschkdll/meschkdll.c",
                mw_version=config.linker_version,
                extra_cflags=["-pooldata off"],
            ),
            Object(
                Matching,
                "REL/meschkdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "endingdll",
        objects={
            Object(NonMatching, "REL/endingdll/ending.c"),
            Object(
                Matching,
                "REL/endingdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "mdpresultdll",
        objects={
            Object(Matching, "REL/mdpresultdll/mdpresult.c"),
            Object(Matching, "REL/mdpresultdll/utility.c"),
            Object(
                Matching,
                "REL/mdpresultdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "m612dll",
        objects={
            Object(Matching, "REL/m612dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m612dll/m612.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m612dll/tables.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m612dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m616dll",
        objects={
            Object(Matching, "REL/m616dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m616dll/m616.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m616dll/init.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m616dll/utility.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m616dll/cpu.c", extra_cflags=["-pooldata off"]),
            Object(
                Matching,
                "REL/m616dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"],
            ),
        },
    ),
    Rel(
        "m618dll",
        objects={
            Object(Matching, "REL/m618dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m618dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m618dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m618dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m621dll",
        objects={
            Object(Matching, "REL/m621dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m621dll/m621.c", extra_cflags=["-pooldata off"]),
            Object(
                Matching,
                "REL/m621dll/gameplay.c",
                extra_cflags=["-pooldata off", "-inline noauto"],
            ),
            Object(
                Matching,
                "REL/m621dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"],
            ),
        },
    ),
    Rel(
        "m635dll",
        objects={
            Object(Matching, "REL/m635dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m635dll/m635.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m635dll/logic.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m635dll/graphics.c", extra_cflags=["-pooldata off"]),
            Object(
                Matching,
                "REL/m635dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"],
            ),
        },
    ),
    Rel(
        "m651dll",
        objects={
            Object(Matching, "REL/m651dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m651dll/m651.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m651dll/players.c", extra_cflags=["-pooldata off"]),
            Object(
                Matching,
                "REL/m651dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"],
            ),
        },
    ),
    Rel(
        "m656DLL",
        objects={
            Object(Matching, "REL/m656DLL/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m656DLL/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m656DLL/matrix.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m656DLL/collision.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m656DLL/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m656DLL/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m657Dll",
        objects={
            Object(Matching, "REL/m657Dll/m657.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/arena.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/player.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/score.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/com.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/hud.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m657Dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m659Dll",
        objects={
            Object(Matching, "REL/m659Dll/globals.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/camera.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/player.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/objects.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/collision.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/scene.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/com.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m659Dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m630dll",
        objects={
            Object(Matching, "REL/m630dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m630dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m630dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m630dll/player.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m630dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m602Dll",
        objects={
            Object(Matching, "REL/m602Dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/support.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/stage.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/effect.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/player.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/ui.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m602Dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m608dll",
        objects={
            Object(Matching, "REL/m608dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/setup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/helpers.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/score.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/sprite.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/ai.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/data.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m608dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m632dll",
        objects={
            Object(Matching, "REL/m632dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m632dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m632dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m632dll/grid.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m632dll/qsort.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m632dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m633dll",
        objects={
            Object(Matching, "REL/m633dll/prolog.c", mw_version=config.linker_version, extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/early.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/input_model.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/audio_helper.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/setup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/model_id.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/middle.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/angle.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/camera.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m633dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m638Dll",
        objects={
            Object(Matching, "REL/m638Dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m638Dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m638Dll/player.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m638Dll/scheduler.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m638Dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m640dll",
        objects={
            Object(Matching, "REL/m640dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m640dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m640dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m640dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m637dll",
        objects={
            Object(Matching, "REL/m637dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m637dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m637dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m637dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m650dll",
        objects={
            Object(Matching, "REL/m650dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m650dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m650dll/game.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m650dll/obstacle.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m650dll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m668DLL",
        objects={
            Object(Matching, "REL/m668DLL/object_start.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/callbacks.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/math_constants.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/object_alloc.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/vectors.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/scale.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/matrix.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/magnitude.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/acos.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/normalize.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/rotation.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/vector_bound.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/startup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/sequence_hooks.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/geometry.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/accessors.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/vector_basis.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/vector_angles.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/data.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m668DLL/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
        },
    ),
    Rel(
        "m670dll",
        objects={
            Object(Matching, "REL/m670dll/prolog.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/sequence.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/seqparam.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/init.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/pattern.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/actor.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/pillar.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/m670dll/cpu.c", extra_cflags=["-pooldata off"]),
            Object(
                Matching,
                "REL/m670dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"],
            ),
        },
    ),
    Rel(
        "mdsingdll",
        objects={
            Object(NonMatching, "REL/mdsingdll/mdsing.c"),
            Object(
                Matching,
                "REL/mdsingdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "openingDll",
        objects={
            Object(Matching, "REL/openingDll/opening.c"),
            Object(
                Matching,
                "REL/openingDll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "optionDll",
        objects={
            Object(NonMatching, "REL/optionDll/option.c"),
            Object(
                Matching,
                "REL/optionDll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "s01Dll",
        objects={
            Object(
                Matching,
                "REL/s01Dll/s01.c",
                mw_version=config.linker_version,
            ),
            Object(
                Matching,
                "REL/s01Dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "s02Dll",
        objects={
            Object(Matching, "REL/s02Dll/s02.c"),
            Object(
                Matching,
                "REL/s02Dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "s03Dll",
        objects={
            Object(
                Matching,
                "REL/s03Dll/s03.c",
                mw_version=config.linker_version,
            ),
            Object(
                Matching,
                "REL/s03Dll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "mdbankdll",
        objects={
            Object(NonMatching, "REL/mdbankdll/mdbank.c"),
            Object(
                Matching,
                "REL/mdbankdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "miraclebookdll",
        objects={
            Object(NonMatching, "REL/miraclebookdll/miraclebook.c"),
            Object(
                Matching,
                "REL/miraclebookdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "staffdll",
        objects={
            Object(NonMatching, "REL/staffdll/staff.c"),
            Object(
                Matching,
                "REL/staffdll/runtime.c",
                mw_version=config.linker_version,
                extra_cflags=["-DMP6_REL_RUNTIME=1"],
            ),
        },
    ),
    Rel(
        "motchkDll",
        objects={
            Object(
                Matching,
                "REL/motchkDll/motchk.c",
                extra_cflags=["-pool off"],
            ),
        },
    ),
    Rel(
        "mdminidll",
        objects=[
            Object(Matching, "REL/mdminidll/startup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/runtime.c", mw_version=config.linker_version, extra_cflags=["-DMP6_REL_RUNTIME=1", "-pooldata off"]),
            Object(Matching, "REL/mdminidll/camera_copy.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/camera_dispatch.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/camera_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/mode_options.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/mode_entry.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/mode_fade_in.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/active_window_wait.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/window_wait.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_groups_create_4538.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_status.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_object_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sequence_icons_done.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sequence_tick.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_visible_22cb4.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_layer_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_grid_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_grid_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/slot_select_f0a8.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/model_rotation.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/scalar_approach.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/scalar_ease.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/scalar_sine_1484.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/scalar_arc_1ed8.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_sine_1f3ac.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_turn_1918.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_move_15b0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_path_1a58.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/scalar_row_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/rows_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_activate.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/light_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_transition.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_group_reveal.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/secondary_models_create_7048.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_model_reset_df64.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_models_create_90c8.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_model_reset_f52c.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/mode_selection_run_1bcac.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/window_transition_17ef4.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/window_transition_18650.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_models_reset_17894.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/window_audio_callback.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/mode_available_1a4.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_approach_10dcc.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/sprite_row_reveal_7f8c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/window_sprite_reveal.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/object_motion_gate.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_group_place.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/position_offset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_choice_tick_111c0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_place.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_neighbor_search.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_neighbors.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_animation_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/scene_create_16400.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_animation_release.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/player_selection.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/motion_grid_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/layer_model_release.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/secondary_window_message.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/shadow_setup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_visible_234f8.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/sequence_run.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/light_release.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/sprite_bank.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_hide.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/exit_check.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/player_config.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/object_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/secondary_state.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_states.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sequence_state.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sequence_complete.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/sequence_done.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/scene_objects_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/layer_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/animation_select.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/window_kill.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/texture_copy.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/callback_4434.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/callback_478c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/callback_84d4.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/layer_hook.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_20f48.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_220e8.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_21764.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_22c5c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_234a0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_kill_23e6c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/window_create.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/window_close_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/scene_cleanup.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/overlay_cleanup_return.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/camera_transition_start.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/camera_transition_update.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/scene_models_release.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_pair_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_pair_create_alt.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/motion_grid_create.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/model_sprite_reset.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_group_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/character_motion_load.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/character_motion_prepare_81c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_model_create.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/minigame_overlay_enter.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_motion_delay.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/sprite_scale_update.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/sprite_project_position.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/particle_model_create_21fe8.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_trajectory_4be0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/object_scene_reset_5bf4.c", extra_cflags=["-pooldata off"]),
            Object(NonMatching, "REL/mdminidll/model_sprite_layout_899c.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_sprite_update_8c08.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/model_choice_layout_dd00.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/player_models_reveal_68f0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/projected_icons_init_118a0.c", extra_cflags=["-pooldata off"]),
            Object(Matching, "REL/mdminidll/layer_effect_position.c", extra_cflags=["-pooldata off"]),
        ],
    ),
]

if args.mode == "configure":
    check_policy_or_exit(tu_declarations_policy)
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    config.progress_each_module = args.verbose
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
