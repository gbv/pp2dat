# pp2dat

> Process possibly dirtry PICA+ import format to clean PICA Normalized

This script reads PICA+ Import format as emitted by `export` script (based on internal C function `write_pp`) and writes normalized PICA+ (one record per line). The postprocessing of CBS export includes basic error-handling to filter out malformed records. The output is ensured to be valid normalized PICA+. Invalid lines are ignored and reported to STDERR with line number and PPN (if known).

## Usage

Run `make` to compile and test the C script. Then use `./pp2dat` to as filter (read from stdin, write to stdout), e.g.

    cat pica.file | ./pp2dat >pica.dat 2>warnings.log

