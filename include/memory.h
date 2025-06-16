#ifndef MEMORY_H
#define MEMORY_H

void* memory_alloc(uint32_t order);
void memory_free(void* ptr, uint32_t order);
void memory_init();

#endif

// #include <utils/kprint.h>
//  for (int i = 0; i <= MEM_ORDER; i++) {
//		//kprint("%d ", i);
//          void* temp  = alloc(i);
//          void* temp2 = alloc(i);
//          if (!temp || !temp2) {
//              kprint("Cannot Allocate: %d\n", i);
//          }
//          //print_free();
//          dalloc(temp, i);
//          dalloc(temp2, i);
//          //print_free();
//      }
//
//      kprint("Done!\n\n");
//
////void print_free()
//{
//    for (int i = 0; i <= MEM_ORDER; i++) {
//        struct memory_block_s* current = memory.free[i];
//        while (current) {
//            kprint("%p ", current);
//            current = current->next;
//        }
//        kprint("%d \n", i);
//    }
//    kprint("\n");
//}
