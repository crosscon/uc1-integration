import sys

# This scripts outputs a command for sagemath scripts based on PUF output.

def parse_hex_blocks(filename):
    blocks = []
    current_block = []
    reading_block = False

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith("MEMREF Output data:"):
                # Start new block if any
                if reading_block and current_block:
                    blocks.append(''.join(current_block))
                    current_block = []
                reading_block = True
            elif reading_block:
                # Stop reading block on Param line or empty line
                if line.startswith("Param[") or line == "":
                    if current_block:
                        blocks.append(''.join(current_block))
                        current_block = []
                    reading_block = False
                else:
                    # Extract hex bytes from the line
                    # line example: 6b 17 d1 f2 e1 2c 42 47  f8 bc e6 e5 63 a4 40 f2 |k....,BG ....c.@.
                    # Split on '|' and take first part, then split by spaces and filter empty
                    hex_part = line.split('|')[0]
                    hex_bytes = hex_part.split()
                    # join all hex bytes (ignore spacing)
                    current_block.append(''.join(hex_bytes))
        # At EOF, append last block if any
        if reading_block and current_block:
            blocks.append(''.join(current_block))
    return blocks

def build_command(blocks):
    # The order and parameter names based on your example
    param_names = ['-gx', '-gy', '-hx', '-hy', '-COMx', '-COMy', '-Px', '-Py', '-v', '-w']

    if len(blocks) < len(param_names):
        print(f"Error: Expected at least {len(param_names)} hex blocks but got {len(blocks)}")
        sys.exit(1)

    # Static last parameter -n (nonce)
    ############################
    # WARNING! HARDCODED VALUE #
    ############################
    static_n = "0X8899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF0011223344556677"

    # Base command (change path if needed)
    base_cmd = (
        "docker run -it -v $(pwd)/puf_vm1/scripts/proofs/:/mnt sagemath/sagemath sage /mnt/proof_verifier_calc.sage \\"
    )

    # Build lines for each parameter
    lines = []
    for param, hexdata in zip(param_names, blocks):
        lines.append(f"{param} 0x{hexdata} \\")

    # Add static -n param last (no backslash)
    lines.append(f"-n {static_n}")

    return base_cmd + '\n' + '\n'.join(lines)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <input_file>")
        sys.exit(1)

    filename = sys.argv[1]
    blocks = parse_hex_blocks(filename)
    command = build_command(blocks)
    print(command)

