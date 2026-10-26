#!/bin/bash
# Compile every firmware source (no Rack) to find porting errors quickly. Usage: tools/fwbuild.sh [file...]
cd "$(dirname "$0")/.."
mkdir -p build/fw
FLAGS="$EXTRA -std=gnu++14 -O1 -fpermissive -w -DF_CPU=120000000 -Ishim -Isrc/oc"
files="$@"
[ -z "$files" ] && files="src/oc_firmware.cpp shim/arduino.cpp $(find src/oc -name '*.cpp' | sort)"
fail=0
for f in $files; do
  o=build/fw/$(echo $f | tr '/' '_').o
  if ! g++ $FLAGS -c $f -o $o 2> build/fw/err.txt; then
    echo "== $f"; grep -E "error" build/fw/err.txt | head -8; fail=1
  fi
done
exit $fail
