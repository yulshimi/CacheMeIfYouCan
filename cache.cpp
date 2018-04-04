//Name: Phillip Jo
/*
 * University of California, San Diego
 *    
 *    CSE141L Summer 2017
 *
 */

#include "cache.h"


//==============================================================================
// Data Memory Module
//==============================================================================

/** DO NOT MODIFY **/
DataMemoryModule::DataMemoryModule() 
{
   this->resetModule();
}

/** DO NOT MODIFY **/
void DataMemoryModule::resetModule() 
{
   this->dmem.reset();
   this->readCount  = 0;
   this->writeCount = 0;
}

/** DO NOT MODIFY **/
uint8_t* DataMemoryModule::getDataMemoryAt(uint8_t index) 
{
   return dmem.get(index);
}

/** DO NOT MODIFY **/
bool DataMemoryModule::loadDataMemoryWith(char *fileName) 
{
   FILE *fp = fopen(fileName, "r");
   if(!fp) 
   {
      return false;
   }
   uint8_t index = 0;
   char buffer[DATA_WIDTH + 1];
   uint8_t *data = (uint8_t*) malloc(sizeof(uint8_t) * DATA_WIDTH);
   while(fscanf(fp, "%s", buffer) == 1) 
   {
      for(int i = 0; i < DATA_WIDTH; i++) 
      {
         data[i] = buffer[i] - '0';
      }
      if(!dmem.set(index++, data)) 
      {
         return false;
      }
   }
   free(data);
   fclose(fp);
   return true;
}

/** DO NOT MODIFY **/
bool DataMemoryModule::write(uint8_t  index_i
                            ,uint8_t* data_i) 
{
   this->writeCount++;
   return dmem.set(index_i, data_i);
}

/** DO NOT MODIFY **/
void DataMemoryModule::read(uint8_t  index_i
                           ,uint8_t*& data_o) 
{
   this->readCount++;
   data_o = dmem.get(index_i);
}

/** DO NOT MODIFY **/
void DataMemoryModule::getDMemStatistics(uint32_t &readCount
                                        ,uint32_t &writeCount) 
{
   readCount  = this->readCount;
   writeCount = this->writeCount;
}

//==============================================================================
// DM Cache Module
//==============================================================================

/** DO NOT MODIFY **/
L1CacheDM::L1CacheDM() : Cache(Cache::DM) 
{
   /** EMPTY **/
} 


L1CacheDM::~L1CacheDM() 
{
   /** YOUR CODE HERE, if necessary **/
}

/*
 * Reset Cache Memory
 *
 */
void L1CacheDM::reset() 
{
   accessCount = 0;
   hitCount = 0;
   writeCount = 0;
   for(int i=0; i < 32; ++i)
   {
     dm_cache[i].valid = 0;
     dm_cache[i].dirty = 0;
   }
}

/*
 * Access cached data
 *
 */
void L1CacheDM::fetch(uint8_t* addr_i
                     ,uint8_t*  hit_o
                     ,uint8_t* data_o) 
{
  *hit_o = 0;
  uint8_t* cache_index_b = generate_cache_addr(addr_i);
  uint8_t* tag = (uint8_t*)malloc(DM_TAG_WIDTH);
  uint8_t cache_index_d;
  cache_index_d = binaryToDecimal(cache_index_b);
  for(int i=0; i < DM_TAG_WIDTH; ++i)
  {
    tag[i] = addr_i[i];
  }
  if(dm_cache[cache_index_d].valid == 0)
  {
    *hit_o = 0;
  }
  else
  {
    if(isTagTheSame(tag, dm_cache[cache_index_d].tag, DM_TAG_WIDTH))
    {
      for(int i=0; i < 16; ++i)
      {
        data_o[i] = dm_cache[cache_index_d].data[i];
      }
      *hit_o = 1;
      ++hitCount;
    }
    else
    {
      *hit_o = 0;
    }
  }
  ++accessCount;
  free(cache_index_b);
  free(tag);
}

/*
 * Update cache
 *
 */
void L1CacheDM::update(uint8_t* addr_i, uint8_t* data_i) 
{
  uint8_t* cache_index_b = generate_cache_addr(addr_i);
  uint8_t cache_index_d = binaryToDecimal(cache_index_b);
  for(int i=0; i < 16; ++i)
  {
    dm_cache[cache_index_d].data[i] = data_i[i];
  }  
  uint8_t* tag = (uint8_t*)malloc(DM_TAG_WIDTH);
  for(int i=0; i < DM_TAG_WIDTH; ++i)
  {
    tag[i] = addr_i[i];
  }
  dm_cache[cache_index_d].valid = 1;
  dm_cache[cache_index_d].dirty = 1;
  for(int i=0; i < DM_TAG_WIDTH; ++i)
  {
    dm_cache[cache_index_d].tag[i] = tag[i];
  }
  ++writeCount;
  free(cache_index_b);
  free(tag);
}

/*
 * Deep copy i-th cache row
 *
 */
void L1CacheDM::peak(uint8_t index, void* row) 
{
  ((DM_Row*)row)->valid = dm_cache[index].valid;
  ((DM_Row*)row)->dirty = dm_cache[index].dirty;
  for(int i=0; i < DM_TAG_WIDTH; ++i)
  {
    ((DM_Row*)row)->tag[i] = dm_cache[index].tag[i];
  }
  for(int i=0; i < 16; ++i)
  {
    ((DM_Row*)row)->data[i] = dm_cache[index].data[i];
  }
}

void L1CacheDM::getStatistics(uint32_t &accessCount
                             ,uint32_t &hitCount
                             ,uint32_t &writeCount) 
{
  accessCount = this->accessCount;
  hitCount = this->hitCount;
  writeCount = this->writeCount;  
}


//==============================================================================
// 2-Way Cache Module
//==============================================================================

/** DO NOT MODIFY **/
L1CacheTW::L1CacheTW() : Cache(Cache::TW) 
{}
 
L1CacheTW::~L1CacheTW() 
{
   /** YOUR CODE HERE, if necessary **/
}

/*
 * Reset cache memory
 *
 */
void L1CacheTW::reset() 
{
  for(int i=0; i < 16; ++i)
  {
    tw_cache[i].dirty1 = 0;
    tw_cache[i].dirty2 = 0;
    tw_cache[i].valid1 = 0;
    tw_cache[i].valid2 = 0;
    tw_cache[i].LRU = 1;
  }
  this->accessCount = 0;
  this->hitCount = 0;
  this->writeCount = 0;
}

/*
 * Access cached data
 *
 */
void L1CacheTW::fetch(uint8_t* addr_i
                     ,uint8_t*  hit_o
                     ,uint8_t* data_o) 
{
  *hit_o = 0;
  uint8_t* cache_index_b = generate_cache_addr(addr_i);
  uint8_t cache_index_d = binaryToDecimal(cache_index_b);
  uint8_t* tag = (uint8_t*)malloc(TW_TAG_WIDTH);
  for(int i=0; i < TW_TAG_WIDTH; ++i)
  {
    tag[i] = addr_i[i];
  }
  if(tw_cache[cache_index_d].valid1 == 1)
  {
    if(isTagTheSame(tag, tw_cache[cache_index_d].tag1, TW_TAG_WIDTH))
    {
      *hit_o = 1;
      ++hitCount; 
      for(int i=0; i < 16; ++i)
      {
        data_o[i] = tw_cache[cache_index_d].data1[i];
      }     
    }
  }
  if(tw_cache[cache_index_d].valid2 == 1)
  {
    if(isTagTheSame(tag, tw_cache[cache_index_d].tag2, TW_TAG_WIDTH))
    {
      *hit_o = 1;
      ++hitCount; 
      for(int i=0; i < 16; ++i)
      {
        data_o[i] = tw_cache[cache_index_d].data2[i];
      }     
    }
  }
  ++accessCount;
  if(*hit_o != 1)
  {
    *hit_o = 0;
  }
  free(cache_index_b);
  free(tag);
}

/*
 * Update cache
 *
 */
void L1CacheTW::update(uint8_t* addr_i, uint8_t* data_i) 
{
  uint8_t* cache_index_b = generate_cache_addr(addr_i);
  uint8_t cache_index_d = binaryToDecimal(cache_index_b);
  uint8_t* tag = (uint8_t*)malloc(TW_TAG_WIDTH);
  for(int i=0; i < TW_TAG_WIDTH; ++i)
  {
    tag[i] = addr_i[i];
  }
  if(isTagTheSame(tw_cache[cache_index_d].tag1, tag, TW_TAG_WIDTH) && tw_cache[cache_index_d].valid1 == 1)
  {
    if(addr_i[7] == 0)
    {
      for(int i=0; i < DATA_WIDTH; ++i)
      {   
        tw_cache[cache_index_d].data1[i] = data_i[i];
      }
    }
    else
    {
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        tw_cache[cache_index_d].data1[i+8] = data_i[i];
      }
    }
    tw_cache[cache_index_d].LRU = 2;
    tw_cache[cache_index_d].valid1 = 1;
    tw_cache[cache_index_d].dirty1 = 1;
  }
  else if(isTagTheSame(tw_cache[cache_index_d].tag2, tag, TW_TAG_WIDTH) && tw_cache[cache_index_d].valid2 == 1)
  {
    if(addr_i[7] == 0)
    {
      for(int i=0; i < DATA_WIDTH; ++i)
      {   
        tw_cache[cache_index_d].data2[i] = data_i[i];
      }
    }
    else
    {
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        tw_cache[cache_index_d].data2[i+8] = data_i[i];
      }
    }
    tw_cache[cache_index_d].LRU = 1;
    tw_cache[cache_index_d].valid2 = 1;
    tw_cache[cache_index_d].dirty2 = 1;
  }
  else
  {
    if(tw_cache[cache_index_d].LRU == 1)
    {
      for(int i=0; i < 16; ++i)
      {
        tw_cache[cache_index_d].data1[i] = data_i[i];
      }
      for(int i=0; i < TW_TAG_WIDTH; ++i)
      {
        tw_cache[cache_index_d].tag1[i] = tag[i];
      }
      tw_cache[cache_index_d].LRU = 2;
      tw_cache[cache_index_d].valid1 = 1;
      tw_cache[cache_index_d].dirty1 = 0;
    }
    else
    {
      for(int i=0; i < 16; ++i)
      {
        tw_cache[cache_index_d].data2[i] = data_i[i];
      }
      for(int i=0; i < TW_TAG_WIDTH; ++i)
      {
        tw_cache[cache_index_d].tag2[i] = tag[i];
      }
      tw_cache[cache_index_d].LRU = 1;
      tw_cache[cache_index_d].valid2 = 1;
      tw_cache[cache_index_d].dirty2 = 0;
    }
  }
  ++writeCount;
  free(cache_index_b);
  free(tag);
}

/*
 * Deep copy i-th cache row
 *
 */
void L1CacheTW::peak(uint8_t index, void* row) 
{
  ((TW_Row*)row)->valid1 = tw_cache[index].valid1;
  ((TW_Row*)row)->dirty1 = tw_cache[index].dirty1;
  for(int i=0; i < TW_TAG_WIDTH; ++i)
  {
    ((TW_Row*)row)->tag1[i] = tw_cache[index].tag1[i];
  }
  for(int i=0; i < 16; ++i)
  {
    ((TW_Row*)row)->data1[i] = tw_cache[index].data1[i];
  }
  ((TW_Row*)row)->valid2 = tw_cache[index].valid2;
  ((TW_Row*)row)->dirty2 = tw_cache[index].dirty2;
  for(int i=0; i < TW_TAG_WIDTH; ++i)
  {
    ((TW_Row*)row)->tag2[i] = tw_cache[index].tag2[i];
  }
  for(int i=0; i < 16; ++i)
  {
    ((TW_Row*)row)->data2[i] = tw_cache[index].data2[i];
  }
  ((TW_Row*)row)->LRU = tw_cache[index].LRU;
}

void L1CacheTW::getStatistics(uint32_t &accessCount
                             ,uint32_t &hitCount
                             ,uint32_t &writeCount) 
{
  accessCount = this->accessCount;
  hitCount = this->hitCount;
  writeCount = this->writeCount;
}

//==============================================================================
// Emulate Cache Access and Memory Write Policy
//==============================================================================

/** DO NOT MODIFY **/
Emulator::Emulator() 
{

   // Comments from Soo: our main.cpp already calls emulator.reset().
   // I think we should call reset() only once either here or on main.cpp
   this->reset();
}

/** DO NOT MODIFY **/
void Emulator::reset() 
{
   this->dmemModule.resetModule();
   this->DMCache.reset();
   this->TWCache.reset();
   this->lastSimulationCacheType   = -1;
   this->lastSimulationWritePolicy = -1;
}

/** DO NOT MODIFY **/
bool Emulator::loadDataMemoryWith(char *fileName) 
{
   return this->dmemModule.loadDataMemoryWith(fileName);
}


void Emulator::writeCacheToFile(uint8_t cacheType, char *file) 
{
  FILE* fptr;
  fptr = fopen(file, "w");
  if(cacheType == 0)
  {
    DM_Row* dm_ptr = DMCache.get_dm_array();
    uint8_t* final_output = (uint8_t*)malloc(20);
    for(int i=0; i < 32; ++i)
    {
      final_output[0] = dm_ptr[i].valid;
      final_output[1] = dm_ptr[i].dirty;
      for(int j=0; j < DM_TAG_WIDTH; ++j)
      {
        final_output[j+2] = dm_ptr[i].tag[j];
      }
      for(int j=0; j < 16; ++j)
      {
        final_output[j+4] = dm_ptr[i].data[j];
      }
      for(int j=0; j < 20; ++j)
      {
        final_output[j] = final_output[j] + '0';
      }
      fprintf(fptr, "%s\n", final_output);
    }
    free(final_output);
  }
  else
  {
    TW_Row* tw_ptr = TWCache.get_tw_array();
    uint8_t* final_output_1 = (uint8_t*)malloc(21);
    uint8_t* final_output_2 = (uint8_t*)malloc(21);
    for(int i=0; i < 16; ++i)
    {
      final_output_1[0] = tw_ptr[i].valid1;
      final_output_1[1] = tw_ptr[i].dirty1;
      for(int j=0; j < TW_TAG_WIDTH; ++j)
      {
        final_output_1[j+2] = tw_ptr[i].tag1[j];
      }
      for(int j=0; j < 16; ++j)
      {
        final_output_1[j+5] = tw_ptr[i].data1[j];
      }
      final_output_2[0] = tw_ptr[i].valid2;
      final_output_2[1] = tw_ptr[i].dirty2;
      for(int j=0; j < TW_TAG_WIDTH; ++j)
      {
        final_output_2[j+2] = tw_ptr[i].tag2[j];
      }
      for(int j=0; j < 16; ++j)
      {
        final_output_2[j+5] = tw_ptr[i].data2[j];
      }
      for(int j=0; j < 21; ++j)
      {
        final_output_1[j] = final_output_1[j] + '0';
        final_output_2[j] = final_output_2[j] + '0';
      }
      fprintf(fptr, "%s %s\n", final_output_1, final_output_2);
    }
    free(final_output_1);
    free(final_output_2);
  }
}


void Emulator::writeDMemToFile(char *file) 
{
  uint8_t* mem_data;
  FILE* fptr;
  fptr = fopen(file, "w");
  for(int i=0; i < 256; ++i)
  {
    mem_data = dmemModule.getDataMemoryAt(i);
    for(int j=0; j < DATA_WIDTH; ++j)
    {
      mem_data[j] = mem_data[j] + '0';
    }
    fprintf(fptr, "%s\n", mem_data);
    free(mem_data);
  }
}


void Emulator::writeStatisticsToFile(char *file) 
{
  char dm[] = "Direct-Memory";
  char tw[] = "2-Way";  
  char wbwa[] = "Write-back, Write-allocate";
  char wtwa[] = "Write-through, Write-allocate";
  FILE* fptr = fopen(file, "w");
  uint8_t cache_type;
  uint8_t cache_w_policy;
  uint32_t cache_access;
  uint32_t cache_hit;
  uint32_t cache_write;
  uint32_t dmem_read;
  uint32_t dmem_write;
  this->getStatistics(cache_type, cache_w_policy, cache_access, cache_hit, cache_write, dmem_read, dmem_write);
  if(cache_type == 0 && cache_w_policy == 0)
  {
    fprintf(fptr, "%s\n", dm);
    fprintf(fptr, "%s\n", wbwa);
  }
  else if(cache_type == 0 && cache_w_policy == 1)
  {
    fprintf(fptr, "%s\n", dm);
    fprintf(fptr, "%s\n", wtwa);
  }
  else if(cache_type == 1 && cache_w_policy == 0)
  {
    fprintf(fptr, "%s\n", tw);
    fprintf(fptr, "%s\n", wbwa);
  }
  else
  {
    fprintf(fptr, "%s\n", tw);
    fprintf(fptr, "%s\n", wtwa);
  }
  fprintf(fptr, "%d\n", cache_access);
  fprintf(fptr, "%d\n", cache_hit);
  fprintf(fptr, "%d\n", cache_write);
  fprintf(fptr, "%d\n", dmem_read);
  fprintf(fptr, "%d\n", dmem_write);
}


/*
 * Return simulation Statistics
 */
void Emulator::getStatistics(uint8_t  &cacheType
                            ,uint8_t  &cacheWritePolicy
                            ,uint32_t &cacheAccessCount
                            ,uint32_t &cacheHitCount
                            ,uint32_t &cacheWriteCount
                            ,uint32_t &dmemReadCount
                            ,uint32_t &dmemWriteCount) 
{
  cacheType = lastSimulationCacheType;
  cacheWritePolicy = lastSimulationWritePolicy;
  if(cacheType == 0)
  {
    DMCache.getStatistics(cacheAccessCount, cacheHitCount, cacheWriteCount);
  }
  else
  {
    TWCache.getStatistics(cacheAccessCount, cacheHitCount, cacheWriteCount);
  }
  dmemModule.getDMemStatistics(dmemReadCount, dmemWriteCount);
}

/*
 * Direct Mapped, Write-back, Write-allocate
 *
 */
bool Emulator::simulateDM_WBWA(char *file) 
{ 
  FILE *fp = fopen(file, "r");
  if(!fp) 
  {
    return false; 
  }

  this->lastSimulationCacheType   = Cache::DM;
  this->lastSimulationWritePolicy = Emulator::WBWA;

  char instrBuffer[INSTR_WIDTH + 1];
  char dataBuffer[DATA_WIDTH + 1];

  while(fscanf(fp, "%s", instrBuffer) == 1) 
  {
    uint8_t hit;
    uint8_t* address = (uint8_t*)malloc(DATA_WIDTH);
    for(int i=0; i < DATA_WIDTH; ++i)
    {
      address[i] = instrBuffer[i+1];
      address[i] = address[i] - '0';
    }
    uint8_t index = binaryToDecimal(address);
    uint8_t* fetched_data = (uint8_t*)malloc(16);
    if(instrBuffer[0] == '1') // load instruction
    {
      DMCache.fetch(address, &hit, fetched_data);
      if(hit != 1) // read miss: read and then update the cache
      {
        uint8_t* temp_data_buffer;
        dmemModule.read(index, temp_data_buffer);
        DM_Row temp_dm_row;
        uint8_t* cache_index_b = DMCache.generate_cache_addr(address);
        uint8_t cache_index_d = binaryToDecimal(cache_index_b);
        DMCache.peak(cache_index_d, &temp_dm_row);
        if(temp_dm_row.dirty == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DM_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_dm_row.tag[i];
            mem_addr_2[i] = temp_dm_row.tag[i];
          }
          for(int i=2; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_dm_row.data[i];
            mem_data_2[i] = temp_dm_row.data[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        } // update memory complete
        uint8_t index_2;
        if(address[7] == 1)
        {
          index_2 = index - 1;
        }
        else
        {
          index_2 = index + 1;
        }
        uint8_t* final_data_buffer = (uint8_t*)malloc(16);
        uint8_t* temp_data_buffer_2;
        dmemModule.read(index_2, temp_data_buffer_2);
        if(index < index_2)
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = temp_data_buffer[i];
            final_data_buffer[i+8] = temp_data_buffer_2[i];
          }
        }
        else
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = temp_data_buffer[i];
            final_data_buffer[i] = temp_data_buffer_2[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        DMCache.setDirtyBit(cache_index_d, 0);
        free(cache_index_b);
        free(temp_data_buffer);
        free(temp_data_buffer_2);
        free(final_data_buffer);
      }
    }
    else // store instruction
    {
      DM_Row temp_dm_row;
      DMCache.fetch(address, &hit, fetched_data);
      fscanf(fp, "%s", dataBuffer);
      uint8_t* read_data = (uint8_t*)malloc(DATA_WIDTH);
      uint8_t* cache_index_b = DMCache.generate_cache_addr(address);
      uint8_t cache_index_d = binaryToDecimal(cache_index_b);
      DMCache.peak(cache_index_d, &temp_dm_row);
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        dataBuffer[i] = dataBuffer[i] - '0';
      }
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        read_data[i] = dataBuffer[i];
      }
      uint8_t* final_data_buffer = (uint8_t*)malloc(16);
      if(hit == 1) // store hit
      {
        if(address[7] == 0)
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = temp_dm_row.data[i+8];
          }
        }
        else
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = dataBuffer[i];
            final_data_buffer[i] = temp_dm_row.data[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        DMCache.setDirtyBit(cache_index_d, 1);
      }
      else // on a store miss
      {
        dmemModule.write(index, read_data);
        if(temp_dm_row.dirty == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DM_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_dm_row.tag[i];
            mem_addr_2[i] = temp_dm_row.tag[i];
          }
          for(int i=2; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_dm_row.data[i];
            mem_data_2[i] = temp_dm_row.data[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        uint8_t* additional_data;
        if(address[7] == 0)
        {
          dmemModule.read(index+1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = additional_data[i];
          }
        }
        else
        {
          dmemModule.read(index-1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = dataBuffer[i];
            final_data_buffer[i] = additional_data[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        DMCache.setDirtyBit(cache_index_d, 1);
        free(additional_data);
      }
      free(read_data);
      free(final_data_buffer);
    }
    free(address);
    free(fetched_data);
  }
  fclose(fp);
  return true;
}


/*
 * Direct Mapped, Write-through, Write-allocate
 *
 */
bool Emulator::simulateDM_WTWA(char *file) 
{
  FILE *fp = fopen(file, "r");
  if(!fp) 
  {
    return false; 
  }

  this->lastSimulationCacheType   = Cache::DM;
  this->lastSimulationWritePolicy = Emulator::WTWA;

  char instrBuffer[INSTR_WIDTH + 1];
  char dataBuffer[DATA_WIDTH + 1];

  while(fscanf(fp, "%s", instrBuffer) == 1) 
  {
    uint8_t hit;
    uint8_t* address = (uint8_t*)malloc(DATA_WIDTH);
    for(int i=0; i < DATA_WIDTH; ++i)
    {
      address[i] = instrBuffer[i+1];
      address[i] = address[i] - '0';
    }
    uint8_t index = binaryToDecimal(address);
    uint8_t* fetched_data = (uint8_t*)malloc(16);
    if(instrBuffer[0] == '1') // load instruction
    {
      DMCache.fetch(address, &hit, fetched_data);
      if(hit != 1) // read miss: read and then update the cache
      {
        uint8_t* temp_data_buffer;
        dmemModule.read(index, temp_data_buffer);
        DM_Row temp_dm_row;
        uint8_t* cache_index_b = DMCache.generate_cache_addr(address);
        uint8_t cache_index_d = binaryToDecimal(cache_index_b);
        DMCache.peak(cache_index_d, &temp_dm_row);
        /*
        if(temp_dm_row.dirty == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DM_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_dm_row.tag[i];
            mem_addr_2[i] = temp_dm_row.tag[i];
          }
          for(int i=2; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_dm_row.data[i];
            mem_data_2[i] = temp_dm_row.data[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        } // update memory complete
        */
        uint8_t index_2;
        if(address[7] == 1)
        {
          index_2 = index - 1;
        }
        else
        {
          index_2 = index + 1;
        }
        uint8_t* final_data_buffer = (uint8_t*)malloc(16);
        uint8_t* temp_data_buffer_2;
        dmemModule.read(index_2, temp_data_buffer_2);
        if(index < index_2)
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = temp_data_buffer[i];
            final_data_buffer[i+8] = temp_data_buffer_2[i];
          }
        }
        else
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = temp_data_buffer[i];
            final_data_buffer[i] = temp_data_buffer_2[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        DMCache.setDirtyBit(cache_index_d, 0);
        free(cache_index_b);
        free(temp_data_buffer);
        free(temp_data_buffer_2);
        free(final_data_buffer);
      }
    }
    else // store instruction
    {
      DM_Row temp_dm_row;
      DMCache.fetch(address, &hit, fetched_data);
      fscanf(fp, "%s", dataBuffer);
      uint8_t* read_data = (uint8_t*)malloc(DATA_WIDTH);
      uint8_t* cache_index_b = DMCache.generate_cache_addr(address);
      uint8_t cache_index_d = binaryToDecimal(cache_index_b);
      DMCache.peak(cache_index_d, &temp_dm_row);
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        dataBuffer[i] = dataBuffer[i] - '0';
      }
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        read_data[i] = dataBuffer[i];
      }
      uint8_t* final_data_buffer = (uint8_t*)malloc(16);
      if(hit == 1) // store hit
      {
        if(address[7] == 0)
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = temp_dm_row.data[i+8];
          }
        }
        else
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = dataBuffer[i];
            final_data_buffer[i] = temp_dm_row.data[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        dmemModule.write(index, read_data);
        DMCache.setDirtyBit(cache_index_d, 0);
      }
      else // on a store miss
      {
        dmemModule.write(index, read_data);
        uint8_t* additional_data;
        if(address[7] == 0)
        {
          dmemModule.read(index+1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = additional_data[i];
          }
        }
        else
        {
          dmemModule.read(index-1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i+8] = dataBuffer[i];
            final_data_buffer[i] = additional_data[i];
          }
        }
        DMCache.update(address, final_data_buffer);
        DMCache.setDirtyBit(cache_index_d, 0);
        free(additional_data);
      }
      free(read_data);
      free(final_data_buffer);
    }
    free(fetched_data);
    free(address);
  }
  fclose(fp);
  return true;
}


/*
 * 2-Way, Write-back, Write-allocate
 *
 */
bool Emulator::simulateTW_WBWA(char *file) 
{
  FILE *fp = fopen(file, "r");
  if(!fp) 
  {
    return false; 
  }

  this->lastSimulationCacheType   = Cache::TW;
  this->lastSimulationWritePolicy = Emulator::WBWA;

  char instrBuffer[INSTR_WIDTH + 1];
  char dataBuffer[DATA_WIDTH + 1];

  while(fscanf(fp, "%s", instrBuffer) == 1) 
  {
    uint8_t hit;
    uint8_t* fetched_data = (uint8_t*)malloc(16);
    uint8_t* mem_address_b = (uint8_t*)malloc(DATA_WIDTH);
    uint8_t mem_address_d;
    uint8_t* final_data_buffer = (uint8_t*)malloc(16);
    for(int i=0; i < DATA_WIDTH; ++i)
    {
      mem_address_b[i] = instrBuffer[i+1];
      mem_address_b[i] = mem_address_b[i] - '0';
    }
    mem_address_d = binaryToDecimal(mem_address_b);
    uint8_t* cache_index_b = TWCache.generate_cache_addr(mem_address_b);
    uint8_t cache_index_d = binaryToDecimal(cache_index_b);
    TW_Row temp_tw_row;
    TWCache.peak(cache_index_d, &temp_tw_row);
    TWCache.fetch(mem_address_b, &hit, fetched_data);
    if(instrBuffer[0] == '1') // load instruction
    {
      if(hit != 1) // Read miss
      {
        if(temp_tw_row.LRU == 1 && temp_tw_row.dirty1 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag1[i];
            mem_addr_2[i] = temp_tw_row.tag1[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data1[i];
            mem_data_2[i] = temp_tw_row.data1[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        if(temp_tw_row.LRU == 2 && temp_tw_row.dirty2 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag2[i];
            mem_addr_2[i] = temp_tw_row.tag2[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data2[i];
            mem_data_2[i] = temp_tw_row.data2[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        uint8_t* temp_data_buffer_1;
        uint8_t* temp_data_buffer_2;
        uint8_t mem_address_d_2;
        if(mem_address_b[7] == 1)
        {
          mem_address_d_2 = mem_address_d - 1;
          dmemModule.read(mem_address_d_2, temp_data_buffer_1);
          dmemModule.read(mem_address_d, temp_data_buffer_2);
        }
        else
        {
          mem_address_d_2 = mem_address_d + 1;
          dmemModule.read(mem_address_d, temp_data_buffer_1);
          dmemModule.read(mem_address_d_2, temp_data_buffer_2);
        }
        for(int i=0; i < DATA_WIDTH; ++i)
        {
          final_data_buffer[i] = temp_data_buffer_1[i];
          final_data_buffer[i+8] = temp_data_buffer_2[i];
        }
        TWCache.update(mem_address_b, final_data_buffer);
        if(temp_tw_row.LRU == 1)
        {
          TWCache.setDirtyBit(cache_index_d, 0, 1);
        }
        else
        {
          TWCache.setDirtyBit(cache_index_d, 0, 2);
        }
        free(temp_data_buffer_1);
        free(temp_data_buffer_2);
      }
    }
    else // store
    {
      fscanf(fp, "%s", dataBuffer);
      uint8_t* read_data = (uint8_t*)malloc(DATA_WIDTH);
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        dataBuffer[i] = dataBuffer[i] - '0';
        read_data[i] = dataBuffer[i];
      }
      if(hit != 1) // store miss
      {
        if(temp_tw_row.LRU == 1 && temp_tw_row.dirty1 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag1[i];
            mem_addr_2[i] = temp_tw_row.tag1[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data1[i];
            mem_data_2[i] = temp_tw_row.data1[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        if(temp_tw_row.LRU == 2 && temp_tw_row.dirty2 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag2[i];
            mem_addr_2[i] = temp_tw_row.tag2[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data2[i];
            mem_data_2[i] = temp_tw_row.data2[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        dmemModule.write(mem_address_d, read_data);
        uint8_t* additional_data;
        if(mem_address_b[7] == 1)
        {
          dmemModule.read(mem_address_d - 1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = additional_data[i];
            final_data_buffer[i+8] = dataBuffer[i];
          }
        }
        else
        {
          dmemModule.read(mem_address_d + 1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = additional_data[i];
          }
        }
        free(additional_data);
        TWCache.update(mem_address_b, final_data_buffer);
        if(temp_tw_row.LRU == 1)
        {
          TWCache.setDirtyBit(cache_index_d, 0, 1);
        }
        else
        {
          TWCache.setDirtyBit(cache_index_d, 0, 2);
        }
      }
      else // store hit
      {
        for(int i=0; i < DATA_WIDTH; ++i)
        {
          final_data_buffer[i] = dataBuffer[i];
          final_data_buffer[i+8] = 0;
        }
        TWCache.update(mem_address_b, final_data_buffer);
      }
    }
    free(fetched_data);
    free(mem_address_b);
    free(final_data_buffer);
  }

  fclose(fp);
  return true;
}
/*
 * 2-Way, Write-through, Write-allocate
 *
 */
bool Emulator::simulateTW_WTWA(char *file) 
{
  FILE *fp = fopen(file, "r");
  if(!fp) 
  {
    return false; 
  }

  this->lastSimulationCacheType   = Cache::TW;
  this->lastSimulationWritePolicy = Emulator::WTWA;

  char instrBuffer[INSTR_WIDTH + 1];
  char dataBuffer[DATA_WIDTH+ 1];

  while(fscanf(fp, "%s", instrBuffer) == 1) 
  {
    uint8_t hit;
    uint8_t* fetched_data = (uint8_t*)malloc(16);
    uint8_t* mem_address_b = (uint8_t*)malloc(DATA_WIDTH);
    uint8_t* final_data_buffer = (uint8_t*)malloc(16);
    uint8_t mem_address_d;
    for(int i=0; i < DATA_WIDTH; ++i)
    {
      mem_address_b[i] = instrBuffer[i+1];
      mem_address_b[i] = mem_address_b[i] - '0';
    }
    mem_address_d = binaryToDecimal(mem_address_b);
    uint8_t* cache_index_b = TWCache.generate_cache_addr(mem_address_b);
    uint8_t cache_index_d = binaryToDecimal(cache_index_b);
    TW_Row temp_tw_row;
    TWCache.peak(cache_index_d, &temp_tw_row);
    TWCache.fetch(mem_address_b, &hit, fetched_data);
    if(instrBuffer[0] == '1') // load instruction
    {
      if(hit != 1) // Read miss
      {
        if(temp_tw_row.LRU == 1 && temp_tw_row.dirty1 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag1[i];
            mem_addr_2[i] = temp_tw_row.tag1[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data1[i];
            mem_data_2[i] = temp_tw_row.data1[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        if(temp_tw_row.LRU == 2 && temp_tw_row.dirty2 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag2[i];
            mem_addr_2[i] = temp_tw_row.tag2[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data2[i];
            mem_data_2[i] = temp_tw_row.data2[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        uint8_t* temp_data_buffer_1;
        uint8_t* temp_data_buffer_2;
        uint8_t mem_address_d_2;
        if(mem_address_b[7] == 1)
        {
          mem_address_d_2 = mem_address_d - 1;
          dmemModule.read(mem_address_d_2, temp_data_buffer_1);
          dmemModule.read(mem_address_d, temp_data_buffer_2);
        }
        else
        {
          mem_address_d_2 = mem_address_d + 1;
          dmemModule.read(mem_address_d, temp_data_buffer_1);
          dmemModule.read(mem_address_d_2, temp_data_buffer_2);
        }
        for(int i=0; i < DATA_WIDTH; ++i)
        {
          final_data_buffer[i] = temp_data_buffer_1[i];
          final_data_buffer[i+8] = temp_data_buffer_2[i];
        }
        TWCache.update(mem_address_b, final_data_buffer);
        if(temp_tw_row.LRU == 1)
        {
          TWCache.setDirtyBit(cache_index_d, 0, 1);
        }
        else
        {
          TWCache.setDirtyBit(cache_index_d, 0, 2);
        }
        free(temp_data_buffer_1);
        free(temp_data_buffer_2);
      }
    }
    else // store
    {
      fscanf(fp, "%s", dataBuffer);
      uint8_t* read_data = (uint8_t*)malloc(DATA_WIDTH);
      for(int i=0; i < DATA_WIDTH; ++i)
      {
        dataBuffer[i] = dataBuffer[i] - '0';
        read_data[i] = dataBuffer[i];
      }
      if(hit != 1) // store miss
      {
         
        if(temp_tw_row.LRU == 1 && temp_tw_row.dirty1 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag1[i];
            mem_addr_2[i] = temp_tw_row.tag1[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data1[i];
            mem_data_2[i] = temp_tw_row.data1[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        if(temp_tw_row.LRU == 2 && temp_tw_row.dirty2 == 1)
        {
          uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < TW_TAG_WIDTH; ++i)
          {
            mem_addr_1[i] = temp_tw_row.tag2[i];
            mem_addr_2[i] = temp_tw_row.tag2[i];
          }
          for(int i=3; i < DATA_WIDTH-1; ++i)
          {
            mem_addr_1[i] = cache_index_b[i+1];
            mem_addr_2[i] = cache_index_b[i+1];
          }
          mem_addr_1[7] = 0;
          mem_addr_2[7] = 1;
          uint8_t mem_index_1 = binaryToDecimal(mem_addr_1);
          uint8_t mem_index_2 = binaryToDecimal(mem_addr_2);
          uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
          uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data2[i];
            mem_data_2[i] = temp_tw_row.data2[i+8];
          }
          dmemModule.write(mem_index_1, mem_data_1);
          dmemModule.write(mem_index_2, mem_data_2);
          free(mem_addr_1);
          free(mem_addr_2);
          free(mem_data_1);
          free(mem_data_2);
        }
        
        dmemModule.write(mem_address_d, read_data);
        uint8_t* additional_data;
        if(mem_address_b[7] == 1)
        {
          dmemModule.read(mem_address_d - 1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = additional_data[i];
            final_data_buffer[i+8] = dataBuffer[i];
          }
        }
        else
        {
          dmemModule.read(mem_address_d + 1, additional_data);
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            final_data_buffer[i] = dataBuffer[i];
            final_data_buffer[i+8] = additional_data[i];
          }
        }
        free(additional_data);
        TWCache.update(mem_address_b, final_data_buffer);
        if(temp_tw_row.LRU == 1)
        {
          TWCache.setDirtyBit(cache_index_d, 0, 1);
        }
        else
        {
          TWCache.setDirtyBit(cache_index_d, 0, 2);
        }
      }
      else // store hit
      {
        for(int i=0; i < DATA_WIDTH; ++i)
        {
          final_data_buffer[i] = dataBuffer[i];
          final_data_buffer[i+8] = 0;
        }
        TWCache.update(mem_address_b, final_data_buffer);
        TWCache.peak(cache_index_d, &temp_tw_row);
        uint8_t* mem_addr_1 = (uint8_t*)malloc(DATA_WIDTH);
        uint8_t* mem_addr_2 = (uint8_t*)malloc(DATA_WIDTH);
        uint8_t* mem_data_1 = (uint8_t*)malloc(DATA_WIDTH);
        uint8_t* mem_data_2 = (uint8_t*)malloc(DATA_WIDTH);
        uint8_t* temp_tag = (uint8_t*)malloc(TW_TAG_WIDTH);
        for(int i=0; i < TW_TAG_WIDTH; ++i)
        {
          temp_tag[i] = mem_address_b[i];
        }
        if(TWCache.isTagTheSame(temp_tag, temp_tw_row.tag1, TW_TAG_WIDTH))
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data1[i];
            mem_data_2[i] = temp_tw_row.data1[i+8];
          }
        }
        else
        {
          for(int i=0; i < DATA_WIDTH; ++i)
          {
            mem_data_1[i] = temp_tw_row.data2[i];
            mem_data_2[i] = temp_tw_row.data2[i+8];
          }
        }
        if(mem_address_b[7] == 0)
        {
          dmemModule.write(mem_address_d, mem_data_1);
          dmemModule.write(mem_address_d + 1, mem_data_2);
        }
        else
        {
          dmemModule.write(mem_address_d - 1, mem_data_1);
          dmemModule.write(mem_address_d, mem_data_2);
        }
        free(mem_addr_1);
        free(mem_addr_2);
        free(mem_data_1);
        free(mem_data_2);
        free(temp_tag);
      }
      free(read_data);
    }
    free(final_data_buffer);
    free(mem_address_b);
    free(fetched_data);
  }

  fclose(fp);
  return true;
}
