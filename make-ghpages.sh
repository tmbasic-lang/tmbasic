#!/bin/bash

# Exit on error
set -e

# Generate help text
./make-help.sh

# Create output directories
mkdir -p bin/ghpages

# Find and copy favicon files
find doc/art/favicon -type f | while read -r file; do
    dest="bin/ghpages/${file#doc/art/favicon/}"
    mkdir -p "$(dirname "$dest")"
    cp -f "$file" "$dest"
done

# Find and copy PNG files from doc/art
find doc/art/ -maxdepth 1 -type f -name "*.png" | while read -r file; do
    dest="bin/ghpages/${file#doc/art/}"
    mkdir -p "$(dirname "$dest")"
    cp -f "$file" "$dest"
done

# Generate the main HTML file
cat doc/help/html/page-template-1.html > bin/ghpages/index.html
echo -n "TMBASIC Programming Language" >> bin/ghpages/index.html
cat doc/help/html/page-template-2.html >> bin/ghpages/index.html
pandoc --from=markdown --to=html README.md >> bin/ghpages/index.html
cat doc/help/html/page-template-3.html >> bin/ghpages/index.html

# Fix URLs in the generated HTML
sed -i 's!https://tmbasic.com/!/!g' bin/ghpages/index.html

# Copy doc-html contents
cp obj/doc-html/* bin/ghpages/ 
