#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define RANGE(c,a,b) (c >= a && c <= b)

int lineNumber = 0;
int fieldCount = 0;
int recordCount = 0;
char PPN[16];

static void printUsage() {
  printf("usage: pp2dat < pica.file > pica.dat");
}

static void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  if (PPN[0]) {
    fprintf(stderr, "%d=%s: ", lineNumber, PPN);
  } else {
    fprintf(stderr, "%d: ", lineNumber);
  }
  vfprintf(stderr, msg, args);
  putc('\n',stderr);
  va_end(args);
}

static int checkSubfields(char *sf) {
  int count=0;

  while(sf[0]) {
    if (sf[0] != '\x1F') {
      warn("expected subfield indicator");
      return 0;
    }
    if (RANGE(sf[1],'0','9') || RANGE(sf[1],'a','z') || RANGE(sf[1],'A','Z')) {
     sf+=2;
    } else {
      warn("invalid subfield indicator: %c", sf[1]);
      return 0;
    }

    count++;
    while (sf[0] != '\x1F' && sf[0]) sf++;
  }

  return count;
}

int main(int argc, char	**argv) {
  char *line = NULL;
  size_t bytes = 0;
  ssize_t length = 0;

  while ((length = getline(&line, &bytes, stdin)) != -1) {
    lineNumber++;

    // trim newline
    if (length > 0 && line[length-1] == '\n') {
      line[--length] = 0;
    }
    
    // ignore empty lines 
    if (length == 0 || line[0] == '#') {
      // should only occurr between records but we allow also within records
      continue;
    }

    // start of record
    if (line[0] == '\x1D') {
      if (recordCount++ && fieldCount) {
        putchar('\n'); // end of previous record
      }
      fieldCount = 0;
      PPN[0] = 0;
      continue;
    }
    
    if (length < 5 || line[0] != '\x1E') {
      warn("fields must start with byte code 1E and a tag");
      continue;
    }

    char *tag = line + 1;
    char *sf = tag + 4;

    if (!RANGE(tag[0],'0','2') || !RANGE(tag[1],'0','9') || !RANGE(tag[2],'0','9') || (!RANGE(tag[3],'A','Z') && tag[3] != '@')) {
      warn("invalid tag %.4s", tag);
      continue;
    }

    if (tag[4] == '/') {
      if (length >= 8 && RANGE(tag[5],'0','9') && RANGE(tag[6],'0','9')) {
        // subfield starts after occurrence
        sf += (tag[0] == '2' && RANGE(tag[7],'0','9')) ? 4 : 3;
      } else {
        warn("invalid tag %.7s", tag);
        continue;
      }
    }

    if (!sf[0] || !sf[1]) {
      warn("empty field %s", tag);
      continue;
    } else if (sf[0] != ' ') {
      warn("missing space after tag");
      continue;
    } else {
      sf[0] = 0; // terminate tag string
      sf++;
    }

    int sfCount = checkSubfields(sf);
    if (sfCount > 0) {
      if (strcmp(tag,"003@") == 0) {
        if (sf[1] != '0' || sfCount != 1 || length > 20) { // TODO: check form of PPN
          warn("malformed 003@: %s", sf);
          continue;
        } else {
          strcpy(PPN, sf+2);
        }
      }
      fieldCount++;
      printf("%s %s\x1E",tag,sf);
    } else {
      continue;
    }
  }    

  if (recordCount > 0 && fieldCount > 0) {
    putchar('\n'); // end of last record
  }

  if (line != NULL)
    free(line);

  return 0;
}
