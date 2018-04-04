//Name: Phillip Jo
/*
 * University of California, San Diego
 *    
 *    CSE141L Summer 2017
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "utility.h"

//==============================================================================
// Direct-mapped L1 Cache object
//
// reset():         initialize the cache
// fetch():         read the data from the cache
// update():        write new data to the cache
// peak():          copy an entire row of the cache at _index_
// getStatistics(): calculate the number of L1cache accesses, hits, and writes
//==============================================================================
class L1CacheDM : public Cache 
{
private:
  uint32_t accessCount;
  uint32_t hitCount;
  uint32_t writeCount;

public:
  L1CacheDM();
  ~L1CacheDM();

  void reset();

  void fetch(uint8_t* addr_i
            ,uint8_t*  hit_o
            ,uint8_t* data_o);

  void update(uint8_t* addr_i
             ,uint8_t* data_i);

  void peak(uint8_t index
           ,void* row);

  void getStatistics(uint32_t &accessCount
                    ,uint32_t &hitCount
                    ,uint32_t &writeCount);
  bool isTagTheSame(uint8_t* tag_1, uint8_t* tag_2, int tag_size)
  {
    for(int i=0; i < tag_size; ++i)
    {
      if(tag_1[i] != tag_2[i])
      {
        return false;
      }
    }
    return true;
  }
  uint8_t* generate_cache_addr(uint8_t* addr_i)
  {
    uint8_t* cache_addr = (uint8_t*)malloc(DATA_WIDTH);
    for(int i=0; i < 3; ++i)
    {
      cache_addr[i] = 0;
    }
    for(int i=3; i < DATA_WIDTH; ++i)
    {
      cache_addr[i] = addr_i[i-1];
    }
    return cache_addr;
  }
  void setDirtyBit(uint8_t index, uint8_t input_bit)
  {
    dm_cache[index].dirty = input_bit;
  }
  DM_Row* get_dm_array()
  {
    return dm_cache;
  }
};

//==============================================================================
// 2-way Set-associative L1 Cache object
//
// reset():         initialize the cache
// fetch():         read the data from the cache
// update():        write new data to the cache
// peak():          copy an entire row of the cache at _index_
// getStatistics(): calculate the number of L1cache accesses, hits, and writes
//==============================================================================
class L1CacheTW : public Cache 
{

   private:
      uint32_t accessCount;
      uint32_t hitCount;
      uint32_t writeCount;
   
   public:
      L1CacheTW();
      ~L1CacheTW();

      void reset();

      void fetch(uint8_t* addr_i
                ,uint8_t*  hit_o
                ,uint8_t* data_o);

      void update(uint8_t* addr_i
                 ,uint8_t* data_i);

      void peak(uint8_t index
               ,void* row);

      void getStatistics(uint32_t &accessCount
                        ,uint32_t &hitCount
                        ,uint32_t &writeCount);
  bool isTagTheSame(uint8_t* tag_1, uint8_t* tag_2, int tag_size)
  {
    for(int i=0; i < tag_size; ++i)
    {
      if(tag_1[i] != tag_2[i])
      {
         return false;
      }
    }
    return true;
  }
  uint8_t* generate_cache_addr(uint8_t* addr_i)
  {
    uint8_t* cache_addr = (uint8_t*)malloc(DATA_WIDTH);
    for(int i=0; i < 4; ++i)
    {
      cache_addr[i] = 0;
    }
    for(int i=4; i < DATA_WIDTH; ++i)
    {
      cache_addr[i] = addr_i[i-1];
    }
    return cache_addr;
  }
  void setDirtyBit(uint8_t index, uint8_t input_bit, uint8_t which)
  {
    if(which == 1)
    {
      tw_cache[index].dirty1 = input_bit;
    } 
    else
    {
      tw_cache[index].dirty2 = input_bit;
    }
  }
  TW_Row* get_tw_array()
  {
    return tw_cache;
  }
};

//===============================================================================
// Data Memory object - DO NOT MODIFY, FULLY IMPLEMENTED
// 
// resetModule():         initialize dMemModule
// loadDataMemoryWith():  load the data dmem.dat
// read():                load the memory data at index_i to data_o
// write():               write data_i to memory at index_i
// getDMemStatistics():   calculate the number of dmem read and write accesses
//===============================================================================
class DataMemoryModule 
{
   private:
      DataMemory dmem;

      uint32_t readCount;
      uint32_t writeCount;

   public:
      DataMemoryModule();

      void resetModule();

      bool loadDataMemoryWith(char *fileName);
      uint8_t* getDataMemoryAt(uint8_t index);

      void read(uint8_t  index_i
               ,uint8_t*& data_o);

      bool write(uint8_t  index_i
                ,uint8_t* data_i);

      void getDMemStatistics(uint32_t &readCount
                            ,uint32_t &writeCount);
};


//===============================================================================================================
// Emulator object
// 
// reset():                 initialize emulator object
// loadDataMemoryWith():    load dmem.dat to dmemModule
//
// simulateDM_WBWA():       direct-mapped, wrie-back, write-allocate
// simulateDM_WTWA():       direct-mapped, wrie-through, write-allocate
// simulateTW_WBWA():       2-way set-associative, wrie-back, write-allocate
// simulateTW_WTWA():       2-way set-associative, wrie-through, write-allocate
//
// writeCacheToFile():      write the last status of cache to a file. It is for more convenient debugging.
// writeDMemToFile():       write the last status of data memory to a file. It is for more convenient debugging.
// writeStatisticsToFile(): write printing values from getStatistics() to a file
// getStatistics():         calculate the numbers of total cache accesses, hits, writes, dmemreads, dmemwrites
//                          it also needs to show what cacheType and WritePolicy the numbers belong to
//===============================================================================================================
class Emulator 
{
   private:
      DataMemoryModule dmemModule;

      L1CacheDM DMCache;
      L1CacheTW TWCache;

      //// lastSimulationCacheType, lastSimulationCacheType
      // keep track of what cache type the code is currently working on
      // useful when running writeStatisticsToFile() and getStatistics()
      uint8_t lastSimulationCacheType;    
      uint8_t lastSimulationWritePolicy;

   public:

      static const uint8_t WBWA = 0;
      static const uint8_t WTWA = 1;

      Emulator();

      void reset();
      
      bool loadDataMemoryWith(char *file);

      bool simulateDM_WBWA(char *file); // direct-mapped, write-back, write-allocate
      bool simulateDM_WTWA(char *file); // direct-mapped, write-through, write-allocate
      bool simulateTW_WBWA(char *file); // 2-way, write-back, write-allocate
      bool simulateTW_WTWA(char *file); // 2-way, write-through, write-allocate

      void writeCacheToFile(uint8_t cacheType, char *fileName);
      void writeDMemToFile(char *fileName);
      void writeStatisticsToFile(char *fileName);

      void getStatistics(uint8_t  &cacheType
                        ,uint8_t  &cacheWritePolicy
                        ,uint32_t &cacheAccessCount
                        ,uint32_t &cacheHitCount
                        ,uint32_t &cacheWriteCount
                        ,uint32_t &dmemReadCount
                        ,uint32_t &dmemWriteCount);

};
