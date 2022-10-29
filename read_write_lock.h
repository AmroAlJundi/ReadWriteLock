/*******************************************************
 * Copyright (c) 2022 Amro Alabsi Aljundi All rights
 * reserved.
 *
 * This file is distributed under MIT license.
 * The complete license agreement is available in the
 * LICENSE file in this repository.
 ********************************************************/

#ifndef _READ_WRITE_LOCK_H
#define _READ_WRITE_LOCK_H

#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <thread>
//! Read-write-lock where multiple readers can read simultaneously, but only a
//! single writer can access the resource.
/*!
 *  Users initiate reading with `start_read` and end it with `end_read`. A
 * writer must initiate reading with `start_write()` and end it with
 * `end_write`. When no writer is writing to the object, multiple readers can
 * access the resource. However, once a writer tries to access the resource, the
 * following takes place: any new read or write attempts will block, the writer
 * will wait for current read requests to finish, and once they do, it will
 * commence writing. Once it finishes, it will notify other writers (if any were
 *  blocked) and then other readers.
 */
namespace ReadWriterLock {
class read_write_lock {
 public:
  //! read_write_locks are not copyable
  read_write_lock(const read_write_lock&) = delete;
  //! read_write_locks are not assignable
  const read_write_lock& operator=(const read_write_lock&) = delete;
  //! Initialize a read_write_lock with no readers and writers
  explicit read_write_lock() : reader_count(0), writer_waiting(0) {}
  //! Start a reading process if no writer is waiting, otherwise waits for
  //! writers to finish writing
  /*!
   * Will only increment reader_count if writer_waiting is 0 at time of checking
   * If writer_waiting is 1, then it will
   */
  void start_read() {
    bool writer_is_writing = writer_is_not_writing();
    if (writer_is_writing != NOT_WRITING) {
      std::unique_lock<std::mutex> lock(read_mutex);
      waiting_to_read.wait(lock, [this] { return writer_is_not_writing(); });
    }
    reader_count++;
  }

  //! End reading
  void end_read() {
    reader_count--;
    waiting_to_write.notify_all();
  }

  //! Start writing
  /*!
   * Stops readers from acquiring resource, and if any readers are reading,
   * waits for them to finish. If there is another writer writing, also waits
   * for it. Once no readers or writers are writing, unblocks and takes the
   * resource.
   */
  void start_write() {
    std::unique_lock<std::mutex> lock(write_mutex);
    writer_waiting += 1;
    if (reader_count > 0) {
      waiting_to_write.wait(lock, [this] { return reader_count <= 0; });
    }
    lock.release();
  }

  //! End writing
  /*!
   * If there is another writer waiting, notifies it, if not, allows the
   * resource to be read.
   */
  void end_write() {
    writer_waiting -= 1;
    waiting_to_write.notify_all();
    write_mutex.unlock();
    waiting_to_read.notify_all();
  }

 private:
  bool writer_is_not_writing() {
    char writer_status = NOT_WRITING;
    char new_writer_status = NOT_WRITING;
    // If no writers are writing/waiting for the resource, writer_status will
    // stay NOT_WRITING. Otherwise, it will change to the number of writers
    writer_waiting.compare_exchange_strong(writer_status, new_writer_status);
    return writer_status == NOT_WRITING;
  }
  std::atomic_int reader_count;
  std::atomic_char writer_waiting;
  std::condition_variable waiting_to_read;
  std::condition_variable waiting_to_write;
  std::mutex read_mutex;
  std::mutex write_mutex;
  const char NOT_WRITING = 0;
};
}  // namespace ReadWriterLock

#endif