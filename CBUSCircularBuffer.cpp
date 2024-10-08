/*
   CBUS Module Library - RasberryPi Pico SDK port
   Copyright (c) Kevin Kimber 2023

   Based on work by Duncan Greenwood
   Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

   This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#include "CBUSCircularBuffer.h"
#include "SystemTick.h"

#include <new>

///
/// A circular buffer class for CBUS messages
///

/// Construct a CBUSCircularBuffer object

CBUSCircularBuffer::CBUSCircularBuffer(uint8_t num_items) : m_full{false},
                                                            m_head{0x0U},
                                                            m_tail{0x0U},
                                                            m_capacity{num_items},
                                                            m_size{0x0U},
                                                            m_highWaterMark{0x0U},
                                                            m_puts{0x0UL},
                                                            m_gets{0x0UL},
                                                            m_overflows{0x0UL}
{
   // Buffer must contain at least one item
   if (num_items > 0)
   {
      // Allocating in constructor, so prevent exception if out of memory
      m_buffer = new (std::nothrow) cbus_frame_buffer_t[num_items];
   }
   else
   {
      m_buffer = nullptr;
   }
}

/// Destroy a CBUSCircularBuffer object instance

CBUSCircularBuffer::~CBUSCircularBuffer()
{
   if (m_buffer)
   {
      delete[] m_buffer;
   }
}

///
/// @brief Indicates if the buffer has one or more items stored in it
///
/// @return true Data is available in the curcular buffer
/// @return false The circular buffer is empty
///
bool CBUSCircularBuffer::available()
{
   return (m_size > 0);
}

///
/// @brief Store an item to the buffer - overwrite oldest item if buffer is full,
/// only called from an interrupt context so we don't need to worry about subsequent interrupts
///
/// @param item CANFrame to store in the circular buffer
///
void __attribute__((section(".RAM"))) CBUSCircularBuffer::put(const CANFrame &item)
{
   if (!m_buffer)
   {
      return;
   }

   // Copy the frame into the item buffer and set the buffer insertion timestamp
   m_buffer[m_head]._item = item;
   m_buffer[m_head]._item_insert_time = SystemTick::GetMicros();

   // if the buffer is full, this put will overwrite the oldest item

   if (m_full)
   {
      m_tail = (m_tail + 1) % m_capacity;
      ++m_overflows;
   }

   m_head = (m_head + 1) % m_capacity;
   m_full = m_head == m_tail;
   m_size = size();
   m_highWaterMark = (m_size > m_highWaterMark) ? m_size : m_highWaterMark;
   ++m_puts;
}

///
/// @brief Retrieve the next item available in the circular buffer
///
/// @return CANFrame* Pointer to the next available item in the circular buffer
///
CANFrame *CBUSCircularBuffer::get()
{
   // If no buffer allocated, return nullptr
   CANFrame *p = nullptr;
   
   if (!m_buffer)
   {
      return p;
   }

   // should always call ::available first to avoid returning null pointer

   // protect against changes to the buffer by suspending interrupts

   if (m_size > 0)
   {
      p = &m_buffer[m_tail]._item;
      m_full = false;
      m_tail = (m_tail + 1) % m_capacity;
      m_size = size();
      ++m_gets;
   }

   return p;
}

///
/// @brief Get the insert time of the current buffer tail item
/// must be called before the item is removed by CBUSCircularBuffer::get
///
/// @return uint32_t Insertion time, in microseconds since boot
///
uint32_t CBUSCircularBuffer::insert_time()
{
   if (!m_buffer)
   {
      return 0x0UL;
   }

   return (m_buffer[m_tail]._item_insert_time);
}

///
/// @brief Peek at the next item in the circular buffer without removing it
///
/// @return CANFrame* Pointer to the next item in the circular buffer
///
CANFrame *CBUSCircularBuffer::peek(void)
{
   // You should always call ::available first to avoid this
   if (!m_buffer || m_size == 0)
   {
      return nullptr;
   }

   return (&m_buffer[m_tail]._item);
}

///
/// @brief Clear all items in the circular buffer
///
void CBUSCircularBuffer::clear(void)
{
   m_head = 0;
   m_tail = 0;
   m_full = false;
   m_size = 0;
}

///
/// @brief Get the high water mark of the circular buffer
///
/// @return uint8_t maximum number of items seen in the circular buffer
///
uint8_t CBUSCircularBuffer::highWaterMark(void)
{
   return m_highWaterMark;
}

///
/// @brief Determine if the circular buffer is full
///
/// @return true if the circular buffer is full
/// @return false if the circular buffer is not full
///
bool CBUSCircularBuffer::full(void)
{
   return m_full;
}

///
/// @brief Recalculate the number of items in the buffer
///
/// @return uint8_t Number of items currently in the circular buffer
///
uint8_t CBUSCircularBuffer::size(void)
{
   uint8_t size = m_capacity;

   if (!m_full)
   {
      if (m_head >= m_tail)
      {
         size = m_head - m_tail;
      }
      else
      {
         size = m_capacity + m_head - m_tail;
      }
   }

   m_size = size;
   return m_size;
}

///
/// @brief Determines if the circular buffer is empty
///
/// @return true if the circular buffer is empty
/// @return false if the circular buffer is not empty
///
bool CBUSCircularBuffer::empty(void)
{
   return (!m_full && (m_head == m_tail));
}

///
/// @brief Determines the number of free entries left in the circular buffer
///
/// @return uint8_t number of entries left in the circular buffer
///
uint8_t CBUSCircularBuffer::free_slots(void)
{
   return (m_capacity - m_size);
}

///
/// @brief Retrieve the number of insertions into the circular buffer
///
/// @return uint32_t number of insertions
///
uint32_t CBUSCircularBuffer::puts(void)
{
   return m_puts;
}

///
/// @brief Retrieve the number of retrievals from the circular buffer
///
/// @return uint32_t number of retrievals
///
uint32_t CBUSCircularBuffer::gets(void)
{
   return m_gets;
}

///
/// @brief Retrieve the number of times the circular buffer overflowed
///
/// @return uint32_t number of circular buffer overflows
///
uint32_t CBUSCircularBuffer::overflows(void)
{
   return m_overflows;
}