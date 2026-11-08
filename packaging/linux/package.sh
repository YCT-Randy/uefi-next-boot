#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
build_dir="${1:-$repo_root/build}"
output_dir="$repo_root/dist"
cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DCPACK_GENERATOR=DEB
cmake --build "$build_dir" --parallel
cpack --config "$build_dir/CPackConfig.cmake" --output-directory "$output_dir"
echo "Debian 套件已產生於 $output_dir/"
