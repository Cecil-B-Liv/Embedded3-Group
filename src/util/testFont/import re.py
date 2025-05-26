# Collect all character arrays into a single 2D array
# First, determine the maximum height (number of rows) across all characters
max_height = max(len(char["bitmap"]) for char in font_array_raw)

# Prepare a list of arrays with zero padding to equal height
char_arrays = []
encoding_map = []

for char in font_array_raw:
    encoding = char["encoding"]
    bitmap = char["bitmap"]
    hex_values = [int(row, 16) for row in bitmap]
    padded_values = hex_values + [0x0000] * (max_height - len(hex_values))
    char_arrays.append(padded_values)
    encoding_map.append(encoding)

# Format as a single 2D array
formatted_combined_array = [
    f"    {{{', '.join(f'0x{val:04X}' for val in row)}}}, // U+{encoding:04X} '{chr(encoding) if 32 <= encoding <= 126 else ''}'"
    for row, encoding in zip(char_arrays, encoding_map)
]

# Compose C file
combined_c_content = f"""\
#include <stdint.h>

// Combined font data, {max_height} rows per character
const uint16_t font_data[][{max_height}] = {{
{chr(10).join(formatted_combined_array)}
}};
"""

# Save to file
combined_c_file_path = "/mnt/data/combined_font_data_10.c"
with open(combined_c_file_path, "w") as f:
    f.write(combined_c_content)

combined_c_file_path
