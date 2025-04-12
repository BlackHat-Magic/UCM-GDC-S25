#!/bin/bash

# Name of the output file
output_file="output.txt"

# Remove the output file if it already exists
[ -f "$output_file" ] && rm "$output_file"

# Generate the tree structure excluding 'assets' and 'build'
# The -I option for tree ignores patterns passed as a | separated list.
echo '```text' > "$output_file"
tree --prune -I "assets|build" >> "$output_file"
echo '```' >> "$output_file"

# Append two newlines to separate the sections
echo -e "\n" >> "$output_file"

# Find all .cpp and .h files, excluding those in "assets" and "build" folders.
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
  ! -path "./build/*" ! -path "./assets/*" \
  | sort | while read -r file; do
    # Append the file name header
    echo "\`$file\`:" >> "$output_file"
    # Append the start of a code block, with c++ highlighting
    echo '```c++' >> "$output_file"
    # Append the file contents
    cat "$file" >> "$output_file"
    # Close the code block
    echo '```' >> "$output_file"
    # Separate each file section with a newline
    echo "" >> "$output_file"
done

