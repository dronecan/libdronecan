#!/usr/bin/env python
import sys, os, shutil, struct
import intelhex

def combine_hex(filename1, filename2, output):
    ih1 = intelhex.IntelHex(filename1)
    ih2 = intelhex.IntelHex(filename2)
    baseaddress1 = ih1.minaddr()
    endaddress1 = ih1.maxaddr()
    baseaddress2 = ih2.minaddr()
    endaddress2 = ih2.maxaddr()
    if baseaddress2 > endaddress1:
        ih1.puts(baseaddress2, ih2.tobinstr())
        ih1.write_hex_file(output)
    elif baseaddress1 > endaddress2:
        ih2.puts(baseaddress1, ih1.tobinstr())
        ih2.write_hex_file(output)
    else:
        print('Error: overlapping hex files')
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Usage: %s output.hex file1.hex file2.hex', sys.argv[0])
        sys.exit(1)
    output = sys.argv[1]
    filenames = sys.argv[2:]
    combine_hex(filenames[0], filenames[1], output)
