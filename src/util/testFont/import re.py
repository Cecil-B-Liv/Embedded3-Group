import re


def reverse_bits_10(value):
    bits_10 = value & 0x03FF  # Only use bottom 10 bits
    return int(f"{bits_10:010b}"[::-1], 2)

# Paths
input_path = "/Users/milkyway/Documents/GitHub/Embedded3-Group/src/util/testFont/Minecraft_font_generated2.h"
output_path = "/Users/milkyway/Documents/GitHub/Embedded3-Group/src/util/testFont/Minecraft_font_fix2.h"

# Read file
with open(input_path, "r") as f:
    content = f.read()

# Match 16-bit hex values (e.g., 0xABCD)
hex_pattern = re.compile(r"0x([0-9A-Fa-f]{4})")
matches = hex_pattern.findall(content)

# Reverse only 10-bit portion
inverted_words = [f"0x{reverse_bits_10(int(h, 16)):04X}" for h in matches]

# Replace content
new_content = hex_pattern.sub(lambda m: inverted_words.pop(0), content)

# Save result
with open(output_path, "w") as f:
    f.write(new_content)

output_path
