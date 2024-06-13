#include<stdio.h>
#include<stdlib.h>
#include <inttypes.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#define COUNT_RECORDS 10
#define MAXSIZE 256
#define FNAME "file"
struct record {
char name[80];
char address[80];
uint8_t semester;
};

FILE *fdopen(int handle, char *mode);

  struct record rec;
  struct record REC_SAV;
  struct record REC_NEW;
  int numRecord;
  int firstPosition, secondPosition;

  struct record records[COUNT_RECORDS] = {
       {"John Smith", "123 Main St", 3},
       {"Alice Johnson", "456 Elm St", 2},
       {"Michael Williams", "789 Oak St", 4},
       {"Emily Brown", "321 Pine St", 1},
       {"Daniel Davis", "654 Maple St", 3},
       {"Olivia Miller", "987 Cedar St", 2},
       {"David Wilson", "135 Spruce St", 4},
       {"Sophia Taylor", "468 Birch St", 1},
       {"Joseph Anderson", "791 Walnut St", 3},
       {"Emma Thomas", "123 Oak St", 2}
   };

void write_records(struct record *records) {
  FILE* f;
  if ((f = fopen("file", "wb")) == NULL) {
  printf("Не удалось открыть файл");
  exit(1);
  }
  fwrite(records, sizeof(struct record), COUNT_RECORDS, f);
  fclose(f);
}

void write_record(FILE* f, struct record rec) {
fwrite(&rec, sizeof(struct record), 1, f);
}

void print_record(struct record rec) {
  printf("%s ", rec.name);
    printf("%s ", rec.address);
      printf("%d\n", rec.semester);
}

void print_records() {
  FILE* f;
  int i;
  struct record readRecords[COUNT_RECORDS];
  if ((f = fopen("file", "rb")) == NULL) {
  printf("Не удалось открыть файл");
  exit(1);
  }
  printf("List of students: \n");
  size_t count = fread(readRecords, sizeof(struct record), COUNT_RECORDS,f);
  if(count < COUNT_RECORDS) {
    printf("empty\n");
  } else {
  for(i = 0; i < COUNT_RECORDS; i++) print_record(readRecords[i]);
  }
  fclose(f);
}


struct record readCurrentRecord(FILE* f) {
  struct record rec;
  fread(&rec, sizeof(struct record),1,f);
  return rec;
}

struct record get(int Rec_No) {
  FILE* f;
  struct record rec;
  f = fopen("file", "rb");
  fseek(f, Rec_No*sizeof(struct record), SEEK_SET);
  rec = readCurrentRecord(f);
  fclose(f);
  return rec;
}


void clear_file() {
  FILE* f = fopen("file", "wb");
  fclose(f);
}

void delay() {
  int value;
  printf("Enter any symbol to continue: ");
  scanf("%d", &value);
}

void menu(){
  printf("1 - write the list of students to a file\n");
  printf("2 - display all students saved in the file\n");
  printf("3 - get()\n");
  printf("q - finish program\n");
}

bool equals(struct record record1, struct record record2) {
  if (strcmp(record1.name, record2.name) != 0) {
         return false;
     }
  if (strcmp(record1.address, record2.address) != 0) {
         return false;
     }
  if (record1.semester != record2.semester) {
         return false;
     }
     return true;
 }

 struct record modificate(struct record rec) {
   int value;
   char c;
   while(1) {
 system("clear");
 print_record(rec);
 printf("'1' - change full name\n");
 printf("'2' - change address\n");
 printf("'3' - change semester\n");
 printf("'4' - cancel\n");
 scanf("%d", &value);
 switch(value) {
     case 1:
     {
       while ((c = getchar()) != '\n' && c != EOF) { }
       fgets(rec.name, 80-1, stdin);
       int length = strlen(rec.name);
       if (length > 0 && rec.name[length - 1] == '\n') {
       rec.name[length - 1] = '\0';
       break;
     }
     case 2:
     {
       while ((c = getchar()) != '\n' && c != EOF) { }
       fgets(rec.address, 80-1, stdin);
       int length = strlen(rec.address);
       if (length > 0 && rec.address[length - 1] == '\n') {
       rec.address[length - 1] = '\0';
   }
       break;
     }
     case 3:
     {
       scanf("%hhd", &rec.semester);
       break;
     }
     case 4: return rec;
     }
     }
   }
 }

void save(struct record rec) {
  int fd = open(FNAME, O_RDWR);
  FILE* f = fdopen(fd, "rb+");
  struct flock lock;
  REC_SAV = rec;
  rec = modificate(rec);
  fseek(f, (numRecord-1)*sizeof(struct record), SEEK_SET);
  firstPosition = ftell(f);
  fseek(f, sizeof(struct record), SEEK_CUR);
  secondPosition = ftell(f);
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = firstPosition;
  lock.l_len = secondPosition - firstPosition;
  if (fcntl(fd, F_SETLK, &lock) < 0)
  printf("Ошибка при:fcntl(fd, F_SETLK, F_WRLCK)(%s)\n",strerror(errno));
  fseek(f, firstPosition, SEEK_SET);
  REC_NEW = readCurrentRecord(f);
  if(!equals(REC_SAV, REC_NEW)) {
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0)
    printf("Ошибка при:fcntl(fd, F_SETLK, F_UNLCK)(%s)\n",strerror(errno));
    printf("Someone changed the post\n");
    sleep(1);
    save(REC_NEW);
  } else {
    fseek(f, firstPosition, SEEK_SET);
    write_record(f, rec);
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0)
    printf("Ошибка при:fcntl(fd, F_SETLK, F_UNLCK)(%s)\n",strerror(errno));
    delay();
  }
  fclose(f);
}



void menu2(struct record rec) {
  int value;
  printf("'1' - modificate record\n");
  printf("'2' - cancel\n");
  scanf("%d",&value);
  switch(value) {
    case 1:
    {
      save(rec);
      break;
    }
    case 2: break;
  }
}


int main() {
  char c;
  clear_file();
   while(1){
     menu();
     c = getchar();
     switch(c) {
       case '1':
       {
           write_records(records);
           break;
       }
       case '2':
       {
         system("clear");
         print_records();
         delay();
         break;
       }
       case '3':
       {
         system("clear");
         printf("enter the number of the student information about whom you want to receive: ");
         scanf("%d", &numRecord);
         rec = get(numRecord-1);
         print_record(rec);
         menu2(rec);
         break;
       }
       case 'q': exit(0);
     }
     system("clear");
   }
  return 0;
}