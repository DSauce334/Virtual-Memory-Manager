#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
//sets up defining values for size of the programs
#define FRAME_SIZE 256
#define PAGE_SIZE 256
#define PHYSSIZE 256
#define PTSIZE 256
#define TLB_SIZE 16
#define OFFMASK 0xFF

// record of page table
typedef struct pagerec
{
  int dirty;
  // index can be used to infer the page number instead of a explicitly declared page field. Added for clarity purpose.
  int page;
  int frame;
} Record;

// record of page table
typedef struct virtframe
{
  int dirty;
  char content[FRAME_SIZE];
} Frame;

typedef struct virttlbrec
{
  int dirty;
  int page;
  int frame;
} TLB_Record;

// circular array structure
typedef struct virttlb
{
  TLB_Record records[TLB_SIZE];
  int tail;
} TLB;
//sets up list of functions in header file for the sake of keeping main file cleaner
char *getline(FILE* fp);
int pagetablesearch(Record *pagetab, int size, int page);
int availableframe(Frame *physmem, int size);
int physmemcopy(Frame *physmem, int size, char *buffer);
int availablerecord(Record *pagetab, int size);
void pagetableupdate(Record *pagetab, int size, int pagenum, int frame);
void pagetableprint(Record *pagetab, int size);
void frameprint(Frame frame);
void tlbprint(TLB tlb);
void tlbupdate(TLB *tlb, int page, int frame);
int tlbsearch(TLB tlb, int page);
int physaddrtrans(int frame, int offset);