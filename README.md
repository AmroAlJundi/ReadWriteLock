# ReadWriteLock: header-only read-write-lock for C++

Header-only read-writer-lock (a.k.a. single-writer lock, multi-reader lock, push lock, or MRSW lock) class for C++. The class is cross-platform with no OS dependencies. Implemented with minimal locking.

Requirements:
- C++11

## Lock behavior
A single lock can be read by an unlimited number of objects. However, only one object can be writing to it. Whenever a writer wants to use the lock, it will prevent new readers from reading and will wait for current readers to finish reading. Then, it will take the lock preventing other readers and writers from taking it. Once it is done writing, it will notify other writers waiting for the lock and as we well as other readers waiting for it. 

## Usage

The following code shows a reader and a writer function:
```c++
ReadWriterLock::read_write_lock lock;

void reader(){
    lock.start_read();
    /// Multiple readers can do this simultaneously
    lock.end_read();
}

void writer(){
    lock.start_write();
    /// Only one writer can be here
    lock.end_writer();
}
```

