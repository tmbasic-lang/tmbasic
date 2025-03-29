#!/bin/bash
set -e

OUTFILE=bin/license.dat
rm -f $OUTFILE

# Create Table of Contents
echo "THIRD PARTY NOTICES" >> $OUTFILE
echo >> $OUTFILE

# Create temp file for libstdc++ license
TEMP_GCC_LICENSE=$(mktemp)
cat "doc/licenses/gcc/GPL-3" "doc/licenses/gcc/copyright" > "$TEMP_GCC_LICENSE"

# Array of license data: "name" "path_to_license_file"
licenses=(
    "abseil"     "doc/licenses/abseil/LICENSE"
    "boost"      "doc/licenses/boost/LICENSE_1_0.txt"
    "fmt"        "doc/licenses/fmt/LICENSE.rst"
    "immer"      "doc/licenses/immer/LICENSE"
    "libstdc++"  "$TEMP_GCC_LICENSE"
    "mpdecimal"  "doc/licenses/mpdecimal/LICENSE.txt"
    "musl"       "doc/licenses/musl/COPYRIGHT"
    "nameof"     "doc/licenses/nameof/LICENSE.txt"
    "ncurses"    "doc/licenses/ncurses/COPYING"
    "scintilla"  "doc/licenses/scintilla/License.txt"
    "tmbasic"    "LICENSE"
    "turbo"      "doc/licenses/turbo/COPYRIGHT"
    "tvision"    "doc/licenses/tvision/COPYRIGHT"
    "utf8proc"   "doc/licenses/utf8proc/LICENSE.md"
    "microtar"   "doc/licenses/microtar/LICENSE"
)

# Generate Table of Contents
for ((i=0; i<${#licenses[@]}; i+=2)); do
    name="${licenses[i]}"
    echo "- ${name}" >> $OUTFILE
done

# Add space after TOC
echo >> $OUTFILE

# Function to append a license to the output file
append_license() {
    local name=$1
    local file=$2
    
    echo "===== $name" >> $OUTFILE
    cat "$file" >> $OUTFILE
    echo >> $OUTFILE
    echo >> $OUTFILE
}

# Process all licenses
for ((i=0; i<${#licenses[@]}; i+=2)); do
    name="${licenses[i]}"
    file="${licenses[i+1]}"
    append_license "$name" "$file"
done

# Clean up temp file
rm -f "$TEMP_GCC_LICENSE"
