#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "vlib.h"


int main(void)
{
  FILE *fbs, *fa;
  int logaddr, physaddr, pagenum, offset;
  //page table has 2^8=256 entries
  Record pagetab[PTSIZE];
  //physical memory has 256*256 bytes (256 frames, 256 bytes per frame)
  Frame physmemory[PHYSSIZE];
  int i = 0, a, corrframe;
  int pagefaults = 0, transcount = 0, tlbhits = 0;
  char *buffer;
  TLB tlb;
  
  //initialize page table
  for (i=0; i<PTSIZE; i++) {
    pagetab[i].dirty = 0;
  }
  
  //initialize physical memory
  for (i=0; i<PHYSSIZE; i++) {
    physmemory[i].dirty = 0;
  }
  
  //initialize TLB
  tlb.tail = 0;
  for (i=0; i<TLB_SIZE; i++) {
    tlb.records[i].dirty = 0;
  }
  //open backing store bin
  fbs = fopen("BACKING_STORE.bin", "rb"); 
  if (fbs == NULL) {
    printf("BACKING_STORE.bin failed opening, errno = %d\n", errno);
    return 1;
  }
  //open address list file
  fa = fopen("addresses.txt", "rb"); 
  if (fa == NULL) {
    printf("addresses.txt failed opening, errno = %d\n", errno);
    return 1;
  }
  
  buffer = (char*) malloc (sizeof(char)*(FRAME_SIZE));
  
  char *s;
	while ((s = getline(fa))) {
    printf("------------------------\n");
	//counts total number of addresses processed
    transcount++;
    logaddr = atoi(s);
    if (logaddr > 65535) {
      printf("Virtual address space is up to 2^16 = 65536. %d Entered. This is out of bounds.\n", logaddr);
      free(s);
      continue;
    }
    
    printf("Logical address: %d\n", logaddr);
    
    //extract page offset by masking to get 8 lsb from logical address.
    offset = logaddr & OFFMASK;
    //extract page number by shift 8 bits to the right and masking, leaving 8 msb are remaining.
    pagenum = (logaddr >> 8) & OFFMASK;
    printf("\tPage number: %-10d\n\tPage offset: %d\n", pagenum, offset);
    
    corrframe = tlbsearch(tlb, pagenum);
	//confirming TLB Hit
    if (corrframe >= 0) {
      printf("\tTLB hit.\n");
      tlbhits++;
	  //confirming TLB Fault
    } else {
      printf("\tTLB fault.\n");
      corrframe = pagetablesearch(pagetab, PTSIZE, pagenum);
      //confirming Page Table hit
      if (corrframe >= 0) {
        printf("\tPage table hit.\n");
        tlbupdate(&tlb, pagenum, corrframe);
      } 
      //confirming page table fault
      else {
        printf("\tPage table fault.\n");
        pagefaults++;
        //move indicator to the page in backing store
        fseek(fbs, sizeof(char)*pagenum*(PAGE_SIZE), SEEK_SET);
        //read a page
        a=fread(buffer, sizeof(char)*(PAGE_SIZE), 1, fbs);
    		//fetch page to physical memory
        corrframe = physmemcopy(physmemory, PHYSSIZE, buffer);
        // printf("Write to physical memory #%d\n", corresponding_frame);
        // print_frame(physical_memory[corresponding_frame]);
        // printf("\n");
        //updates page table
        pagetableupdate(pagetab, PTSIZE, pagenum, corrframe);
        tlbupdate(&tlb, pagenum, corrframe);
      }
    }
      
    // print_page_table(page_table, PAGE_TABLE_SIZE);
    // print_tlb(tlb);
    physaddr = physaddrtrans(corrframe, offset);
    printf("Physical address: %-15d\n Value: %c\n", physaddr, physmemory[corrframe].content[offset]);
    free(s);
    printf("\n");
	}
  //prints out final values and calcuations
  printf("Number of Addresses Tanslated: %d\n", transcount);
  printf("Page fault: %d\n", pagefaults);
  printf("Page fault rate in percent: %f\n", (float) pagefaults/transcount);
  printf("TLB hit: %d\n", tlbhits);
  printf("TLB hit rate in percent: %f\n", (float) tlbhits/transcount);
  //close and ends files and processes with program done
  free(buffer);
  free(s);
  fclose(fbs); 
	return 0;
}
