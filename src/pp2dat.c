#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define RANGE(c,a,b) (c >= a && c <= b)
#define MAX_RECORD_LENGTH 1024*1024
#define MAX_ILNS_LENGTH 1024*4

int lineNumber = 0;
int fieldCount = 0;
int recordCount = 0;
char PPN[16];

size_t recordLength = 0;
char recordBuffer[MAX_RECORD_LENGTH+1];

bool recordHasIlns = false;
size_t ilnsLength = 0;
char ilns[MAX_ILNS_LENGTH];

bool flush = false; // TODO: make CLI option

static void printUsage() {
  printf("usage: pp2dat < pica.file > pica.dat");
}

static void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  if (PPN[0]) {
    fprintf(stderr, "line %d PPN %s: ", lineNumber, PPN);
  } else {
    fprintf(stderr, "line %d: ", lineNumber);
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

static void printField(char *tag, char *sf) {
  if (flush) {
    printf("%s %s\x1E",tag,sf);
  } else {
    size_t sfLength = strlen(sf);
    size_t length = 2 + strlen(tag) + sfLength;
  
    if (recordLength + length >= MAX_RECORD_LENGTH) {
      warn("maximum record length exceeded");
      exit(1);
    }
    sprintf(recordBuffer + recordLength, "%s %s\x1E",tag,sf);
    recordLength += length;
  
    // collect ILNs from 101@ unless record has 001@
    if (strcmp(tag,"001@")==0) {
      recordHasIlns = true;
      ilnsLength = 0;
    } else if (!recordHasIlns && strcmp(tag,"101@")==0) {
      if (ilnsLength + sfLength > MAX_ILNS_LENGTH) {
        warn("maximium length of ILNs exceeded");
        exit(1);
      }
      strcpy(ilns + ilnsLength, sf + 2);
      ilnsLength += sfLength - 1;
      ilns[ilnsLength-1] = ',';
    }
  }
}

static void endRecord() {
  if (flush) {
    putchar('\n');
  } else {

    // first print 001@ with collected ILNs
    if (ilnsLength > 0) {
      ilns[ilnsLength-1] = 0;
      printf("001@ \x1F" "0%s\x1E",ilns);
      ilnsLength = 0;
    }

    // print current record
    recordBuffer[recordLength] = 0;
    puts(recordBuffer);

    // start new record
    recordLength = 0;
    recordHasIlns = false;
  }
}
                       
bool checkPPN(char *ppn, size_t length) {
  if (length < 2 || length > 12) return false;

  int sum = 0;
  for (size_t i=0; i<length-1; i++) {
    if (!RANGE(ppn[i],'0','9')) return false;
    sum += (ppn[i]-'0') * (length-i);
  }
  sum = (11 - (sum % 11)) % 11;
  return (sum < 10 ? sum + '0' : 'X') == ppn[length-1];
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
        endRecord();
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
        if (sf[1] != '0' || sfCount != 1 || !checkPPN(sf+2, length-8)) {
          warn("malformed 003@: %s", sf);
          continue;
        } else {
          strcpy(PPN, sf+2);
        }
      }
      fieldCount++;
      printField(tag,sf);
    } else {
      continue;
    }
  }    

  if (recordCount > 0 && fieldCount > 0) {
    endRecord();
  }

  if (line != NULL)
    free(line);

  return 0;
}

