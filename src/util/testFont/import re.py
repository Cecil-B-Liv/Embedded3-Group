import re


def reverse_bits(byte):
    return int(f"{byte:08b}"[::-1], 2)


# Read the file content
input_path = "/Users/milkyway/Documents/GitHub/Embedded3-Group/src/util/testFont/Minecraft_font_up_side.h"
output_path = "/Users/milkyway/Documents/GitHub/Embedded3-Group/src/util/testFont/Minecraft_font_fix2.h"

with open(input_path, "r") as f:
    content = f.read()

# Find all byte values in hex format
byte_pattern = re.compile(r"0x([0-9A-Fa-f]{2})")
matches = byte_pattern.findall(content)

# Invert bits for each hex value
inverted_bytes = [f"0x{reverse_bits(int(b, 16)):02X}" for b in matches]

# Replace the original values with inverted ones
new_content = byte_pattern.sub(lambda m: inverted_bytes.pop(0), content)

# Write to a new file
with open(output_path, "w") as f:
    f.write(new_content)

output_path
