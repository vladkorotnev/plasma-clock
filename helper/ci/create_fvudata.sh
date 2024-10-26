#!/bin/bash

# Directory containing the folders to check
source_dir="./.pio/build"
# Target directory for copied files
target_dir="./webroot/fvudata"

# Create the target directory if it doesn't exist
mkdir -p "$target_dir"

cp "./.pio/build/music-pomf/littlefs.bin" "$target_dir/fs.fvu"
cp "./data/FS_VER" "$target_dir/fs_ver.txt"

# Iterate over each entry in the source directory
for entry in "$source_dir"/*; do
    # Check if the entry is a directory
    if [ -d "$entry" ]; then
        # Define the path to firmware.bin inside the current directory
        firmware_file="$entry/firmware.bin"
        
        # Check if firmware.bin exists in the current directory
        if [ -f "$firmware_file" ]; then
            # Get the name of the directory (basename)
            folder_name=$(basename "$entry")
            
            # Copy and rename firmware.bin to the target directory
            cp "$firmware_file" "$target_dir/$folder_name.avu"
            echo "Copied $firmware_file to $target_dir/$folder_name.avu"
        fi
    fi
done
