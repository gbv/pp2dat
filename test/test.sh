#!/usr/bin/bash

is_equal() {
  if [ "$2" != "$3" ]; then
    diff <(echo "$2") <(echo "$3")
    exit 1
  else
    echo "$1 ok"
  fi
}

warnings=$(./pp2dat < ./test/example.pica 2>&1 >/dev/null)
is_equal "warnings" "$warnings" 'line 14: invalid subfield indicator: .
line 17 PPN 123056789019: empty field 034X
line 20 PPN 99X: invalid tag ABCD
line 21 PPN 99X: invalid tag 012X/AB
line 22 PPN 99X: empty field 234X/00A
line 23 PPN 99X: empty field 234X/123
line 32: malformed 003@: 01230567890123'

pica=$(./pp2dat < ./test/example.pica 2>/dev/null)

is_equal "pica" "$pica" '003@ 0124021A aEin Buchhzum Lesen045B/02 aSpo 1025aBID 200
003@ 07897
003@ 0123056789019021A aok
003@ 099X234X/123 a!
001@ 07,12101@ a7101@ a12
001@ 099101@ a99'
