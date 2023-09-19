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
is_equal "warnings" "$warnings" '14: invalid subfield indicator: .
17=654: empty field 034X
20=999: invalid tag ABCD
21=999: invalid tag 012X/AB
22=999: empty field 234X/00A
23=999: empty field 234X/123'

pica=$(./pp2dat < ./test/example.pica 2>/dev/null)

is_equal "pica" "$pica" '003@ 0123021A aEin Buchhzum Lesen045B/02 aSpo 1025aBID 200
003@ 0789
003@ 0654021A aok
003@ 0999234X/123 a!'
