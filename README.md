# pp2dat

> Process possibly dirty internal PICA+ import format to clean Normalized PICA+ format

This script reads PICA+ as serialized by CBS internal C function `write_pp` and writes [normalized PICA+](https://format.gbv.de/pica/normalized). CBS is a prorietary database management system by OCLC and PICA+ is its internal record format. More information about PICA+ can be found in the German Handbook *[Einführung in die Verarbeitung von PICA-Daten](https://pro4bib.github.io/pica)*. This script is needed because the internal serialization of PICA+ is neither defined openly nor strictly validated. In contrast normalized PICA+ has a short definition (one record per line with line separator `0A`, fields end with `1E`, subfields indicator `1F`) and it is supported by open source tools such as [pica-rs](https://github.com/deutsche-nationalbibliothek/pica-rs). The output of **pp2dat** is ensured to be syntactically valid normalized PICA+. The following is checked:

- basic record structure with fields and subfields
- field tags must match regular expression `^[012][0-9][0-9][A-Z@]$`
- fields must not be empty
- subfield indicators must match regular expression `^[0-9a-zA-Z]$`
- field `003@` must have exactely one subfield `0` with value not longer than 20 characters (PPN)
- maximum record length 1 MiB (restricted by implementation, could be increased if needed)

Invalid input lines are ignored and reported to STDERR with line number, and PPN if known.

The script also collects fields `101@` and prepends field `001@` with a list of ILNs unless the record already contains field `001@`.

## Usage

Run `make` to compile and test the C script. Then use `./pp2dat` to as filter (read from STDIN, write to STDOUT), e.g.

    cat pica.file | ./pp2dat >pica.dat 2>warnings.log

## Development

There is a unit test in directory `test`, automatically run at build. Directly run with:

    make test

## LICENSE

This script and its source code is made available under MIT License.
