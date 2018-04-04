/*
 * University of California, San Diego
 *    
 *    CSE141L Summer 2017
 *
 */

#ifndef _UTILITY_H
#define _UTILITY_H

#include <stdlib.h>
#include <stdint.h>

#define DATA_WIDTH  8
#define INSTR_WIDTH 9

#define DMEM_DEPTH 256

#define DM_OFFSET_WIDTH 1 
#define DM_INDEX_WIDTH  5
#define DM_TAG_WIDTH    2

#define TW_OFFSET_WIDTH 1
#define TW_INDEX_WIDTH  4
#define TW_TAG_WIDTH    3

#define CACHELINE_WIDTH 16


/*
 * struct for a row of direct-mapped cache
 *
 * DO NOT MODIFY
 *
 */
typedef struct DM_Row 
{
   uint8_t valid;
   uint8_t dirty;
   uint8_t tag[DM_TAG_WIDTH];
   uint8_t data[CACHELINE_WIDTH];
} DM_Row;


/*
 * struct for a row of two-way set-associative cache
 *
 * DO NOT MODIFY
 *
 */
typedef struct TW_Row 
{
   uint8_t valid1;
   uint8_t dirty1;
   uint8_t tag1[TW_TAG_WIDTH];
   uint8_t data1[CACHELINE_WIDTH];

   uint8_t valid2;
   uint8_t dirty2;
   uint8_t tag2[TW_TAG_WIDTH];
   uint8_t data2[CACHELINE_WIDTH];

   uint8_t LRU;
} TW_Row;


/*
 * Cache Interface
 *
 * DO NOT MODIFY
 *
 */
class Cache 
{
protected:
  DM_Row *dm_cache;
  TW_Row *tw_cache;
  uint8_t cacheType;
public:
  static const uint8_t DM = 0;
  static const uint8_t TW = 1;

      // constructor of Cache object
      // direct-mapped: 32 rows of DM_Row struct
      // 2-way: 16 rows of TW_Row struct
      Cache(uint8_t type)
      {
         cacheType = type;

         switch(cacheType) 
         {
            case DM: 
              dm_cache = (DM_Row*) malloc(sizeof(DM_Row) * 32); 
              break;
            case TW: 
              tw_cache = (TW_Row*) malloc(sizeof(TW_Row) * 16); 
              break;
            default: 
              break;
         }
      }

      virtual ~Cache() 
      {
         switch(cacheType) 
         {
            case DM: 
              free(dm_cache); 
              break;
            case TW: 
              free(tw_cache); 
              break;
            default: 
              break;
         }
      }

      // reset(): initialize the cache
      virtual void reset() = 0;

      // fetch(): read the data from the cache
      virtual void fetch(uint8_t* addr_i
                        ,uint8_t*  hit_o
                        ,uint8_t* data_o) = 0;

      // update(): write new data to the cache
      virtual void update(uint8_t* addr_i
                         ,uint8_t* data_i) = 0;
      
      // peak(): copy an entire row of the cache at _index_
      virtual void peak(uint8_t index, void* row) = 0;

};


/*
 * Data Memory; 8-bit width
 *
 * DO NOT MODIFY
 *
 */
class DataMemory 
{

   private:
      uint8_t memory[DMEM_DEPTH][DATA_WIDTH];

      bool assertIndex(uint8_t index) 
      {
         return (index >= 0 && index < DMEM_DEPTH) ? true : false;
      }

      bool assertData(uint8_t *data) 
      {
         for(int i = 0; i < DATA_WIDTH; i++) 
         {
            if(data[i] != 0 && data[i] != 1) 
            {
               return false;
            }
         }
         return true;
      }

   public:
      DataMemory()  {};
      ~DataMemory() {};
      
      void reset() 
      {
         for(int i = 0; i < DMEM_DEPTH; i++) 
         {
            for(int j = 0; j < DATA_WIDTH; j++) 
            {
               memory[i][j] = 0;
            }
         }
      }

      bool set(uint8_t index, uint8_t *data) 
      {
         
         if(!this->assertIndex(index) || !this->assertData(data)) 
         {
            return false;
         }

         for(int i = 0; i < DATA_WIDTH; i++) 
         {
            this->memory[index][i] = data[i];
         }

         return true;
      }

      uint8_t* get(uint8_t index) 
      {

         if(!this->assertIndex(index)) 
         {
            return NULL;
         }

         uint8_t *data = (uint8_t*) malloc(sizeof(uint8_t) * DATA_WIDTH);

         for(int i = 0; i < DATA_WIDTH; i++) 
         {
            data[i] = this->memory[index][i];
         }

         return data;
      }

};

/*
 * Convert unsigned 8-bit binary to unsigned 8-bit  integer
 *
 * DO NOT MODIFY
 *
 */
inline uint8_t binaryToDecimal(uint8_t* binary) 
{
   return (1<<7) * binary[0] + 
          (1<<6) * binary[1] + 
          (1<<5) * binary[2] + 
          (1<<4) * binary[3] + 
          (1<<3) * binary[4] + 
          (1<<2) * binary[5] + 
          (1<<1) * binary[6] + 
                   binary[7];   
}

/*
 * Convert unsigned 8-bit integer to unsigned 8-bit binary
 *
 * DO NOT MODIFY
 *
 */
inline uint8_t* decimalToBinary(uint8_t decimal) 
{

   uint8_t *binary = (uint8_t*) malloc (sizeof(uint8_t) * DATA_WIDTH);
   uint8_t mask = 1;
   for(int i = 0, j = DATA_WIDTH-1; i < DATA_WIDTH; i++, j--) 
   {
      binary[j] = (decimal & mask) != 0 ? 1 : 0;
      mask <<= 1;
   }

   return binary;
}

#endif