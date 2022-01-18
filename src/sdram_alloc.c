#include "sdram_alloc.h"
#include "stm32746g_discovery_sdram.h"
//#include "ct-head/math.h"

// start of heap = SDRAM base address PLUS length of screen buffer
static uint32_t sdram_heap_ptr = SDRAM_DEVICE_ADDR + 480 * 276 * 4;
static uint32_t sdram_heap_end = SDRAM_DEVICE_ADDR + SDRAM_DEVICE_SIZE;
static uint32_t sdram_zero[32] = {0};

unsigned int nextPowerOf2(unsigned int n) 
{ 
    unsigned count = 0; 
    
    // First n in the below condition 
    // is for the case where n is 0 
    if (n && !(n & (n - 1))) 
        return n; 
    
    while( n != 0) 
    { 
        n >>= 1; 
        count += 1; 
    } 
    
    return 1 << count; 
} 

void* ct_sdram_malloc(size_t size) {
  void* ptr = NULL;
  if (sdram_heap_ptr + size < sdram_heap_end) {
    ptr = (void*)sdram_heap_ptr;
    sdram_heap_ptr += nextPowerOf2(size);
  }
  return ptr;
}

void* ct_sdram_calloc(size_t num, size_t size) {
  size *= num;
  void* ptr = ct_sdram_malloc(size);
  if (ptr) {
    uint32_t addr = (uint32_t)ptr;
    while (size > 32) {
      BSP_SDRAM_WriteData(addr, sdram_zero, 32);
      addr += 32;
      size -= 32;
    }
    if (size > 0) {
      BSP_SDRAM_WriteData(addr, sdram_zero, size);
    }
  }
  return ptr;
}
