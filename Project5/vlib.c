#include "vlib.h"
//fetches a line from the address value
char *getline(FILE* fp)
{
	int len = 0, got = 0, c;
	char *buffer = 0;
  
	while ((c = fgetc(fp)) != EOF) {
		if (got + 1 >= len) {
			len *= 2;
			if (len < 4) len = 4;
			buffer = realloc(buffer, len);
		}
		if (c == '\n') break;
	//confirms addresses are numerical values
    if (c - '0' < 0 || c - '0' > 9) {
      printf("Only numbers are allowed!\n");
      return 0;
    }
		buffer[got++] = c;
	}
	if (c == EOF && !got) return 0;
 
	buffer[got++] = '\0';
	return buffer;
}
//runs through pagetable values
int pagetablesearch(Record *pagetab, int size, int page) {
  int i;
  for (i=0; i<size; i++) {
    if (pagetab[i].dirty == 1 && pagetab[i].page == page) {
      return pagetab[i].frame;
    }
  }
  return -1;
}
//finds available physical memory frame for replacement/placement
int availableframe(Frame *physmem, int size) {
  int i;
  for (i=0; i<size; i++) {
    if (physmem[i].dirty == 0) {
      return i;
    }
  }
  return -1;
}
//copys data to physical memory
int physmemcopy(Frame *physmem, int size, char *buffer) {
  int i = availableframe(physmem, size);
  if (i >= 0) {
    memcpy(physmem[i].content, buffer, FRAME_SIZE);
    physmem[i].dirty = 1;
    return i;
  }
  return -1;
}

int availablerecord(Record *pagetab, int size) {
  int i;
  for (i=0; i<size; i++) {
    if (pagetab[i].dirty == 0) {
      return i;
    }
  }
  return -1;
}
//updates a page table with a change in the page table
void pagetableupdate(Record *page_table, int size, int page_number, int frame) {
  int a = availablerecord(page_table, size);
  if (a >= 0) {
    printf("\tUpdate in page table #%d, page %d, frame %d\n", a, page_number, frame);
    page_table[a].page = page_number;
    page_table[a].frame = frame;
    page_table[a].dirty = 1;
  } else {
    printf("No available record in page table found!\n");
  }
}
//prints out values in the page table when it is replaced
void pagetableprint(Record *page_table, int size) {
  int i;
  for (i=0; i<size; i++) {
    if (page_table[i].dirty == 0) {
      return;
    }
    printf("Page table #%-3d: page %-15d frame %-15d dirty %d\n", i, page_table[i].page, page_table[i].frame, page_table[i].dirty);
  }
}
//prints frame value when replaced/placed
void frameprint(Frame frame) {
  int i;
  for (i=0; i<256; i++) {
    printf("%c", frame.content[i]);
  }
}
//prints tlb value when replacement occurs
void tlbprint(TLB tlb) {
  int i;
  
  printf("TLB tail: %d\n", tlb.tail);
  for (i=0; i<TLB_SIZE; i++) {
    printf("Page: %-10dFrame: %-10dDirty: %d\n", tlb.records[i].page, tlb.records[i].frame, tlb.records[i].dirty);
  }
}
//updates TLB with change in TLB table values
void tlbupdate(TLB *tlb, int page, int frame) {
  tlb->records[tlb->tail].page = page;
  tlb->records[tlb->tail].frame = frame;
  tlb->records[tlb->tail].dirty = 1;
  printf("\tUpdate in TLB #%d: page %d, frame %d\n", tlb->tail, tlb->records[tlb->tail].page, tlb->records[tlb->tail].frame);
  tlb->tail = (tlb->tail + 1) % TLB_SIZE;
}
//scans TLB table to search for values to replace
int tlbsearch(TLB tlb, int page) {
  int i;
  
  for (i=0; i<TLB_SIZE; i++) {
    if (tlb.records[i].dirty == 1) {
      if (tlb.records[i].page == page) {
        return tlb.records[i].frame;
      }
    }
  }
  return -1;
}
//translates physical memory in relation to frame and offset
int physaddrtrans(int frame, int offset) {
  int address = frame;
  // printf("%d\n", address);
  address = address << 8;
  // printf("%d\n", address);
  address = address | offset;
  // printf("%d\n", address);
  return address;
}
