#ifndef REPO_HPP
#define REPO_HPP

#include <assert.h>
#include <iostream>


// Used just to account for the size of vtable pointers.
//class Vtable {
//public:
//  virtual void run() = 0;
//};

template <unsigned long Size>
class RepoHeader {
private:

  enum { MAGIC_NUMBER = 0xCAFEBABE };
  
public:

  enum { Alignment = 2 * sizeof(unsigned long) };
  
  RepoHeader(unsigned long objectSize)
    : _allocated (0),
      _magic (MAGIC_NUMBER),
      _next (nullptr)
  {
    setObjectSize(objectSize);
  }

  inline void setObjectSize(size_t sz) {
    _objectSize = sz;
    _numberOfObjects = ((Size-sizeof(*this)) / _objectSize);
    tprintf::tprintf("setting object size to @, numObjects = @\n", sz, _numberOfObjects);
  }

  inline auto getObjectSize() const {
    return _objectSize;
  }

  inline auto getNumberOfObjects() const {
    return _numberOfObjects;
  }
  
  inline void setNext(RepoHeader * p) {
    _next = p;
  }

  inline auto getNext() const {
    return _next;
  }

  inline auto getAllocated() const {
    return _allocated;
  }

  inline void decAllocated() {
    _allocated--;
  }
  
  inline void incAllocated() {
    _allocated++;
  }
  
private:
  unsigned long _objectSize;
  unsigned long _numberOfObjects;
  unsigned long _allocated;  // total number of objects allocated so far.
  const unsigned long _magic;
  RepoHeader * _next;
  unsigned long _dummy; // for alignment

public:
  
  inline size_t getBaseSize() {
    assert(isValid());
    return _objectSize;
  }

  inline bool isValid() const {
    // return true;
    return (_magic == MAGIC_NUMBER);
  }
  
};

// The base for all object sizes of repos.
template <unsigned long Size>
class Repo : public RepoHeader<Size> {
public:
  
  Repo(unsigned long objectSize)
    : RepoHeader<Size>(objectSize)
  {
    static_assert(sizeof(*this) == Size, "Something has gone terribly wrong.");
  }

  inline constexpr auto getNumberOfObjects() const {
    return RepoHeader<Size>::getNumberOfObjects();
  }
  
  inline bool isFull() {
    return (RepoHeader<Size>::getAllocated() == getNumberOfObjects());
  }

  inline bool isEmpty() {
    return (RepoHeader<Size>::getAllocated() == 0);
  }

  inline void * malloc(size_t sz) {
    //    std::cout << "this = " << this << std::endl;
    assert(RepoHeader<Size>::isValid());
    assert (sz <= RepoHeader<Size>::getObjectSize());
    if (sz < RepoHeader<Size>::getObjectSize()) {
      tprintf::tprintf("OK WAT @ should be @\n", sz, RepoHeader<Size>::getObjectSize());
    }
    void * ptr;
    if (!isFull()) {
      ptr = &_buffer[RepoHeader<Size>::getAllocated() * RepoHeader<Size>::getObjectSize()];
      assert(inBounds(ptr));
      RepoHeader<Size>::incAllocated();
      memset(ptr, 'X', RepoHeader<Size>::getObjectSize()); // FIXME
    } else {
      //      std::cout << "out of objects: _allocated = " << RepoHeader<Size>::_allocated << std::endl;
      ptr = nullptr;
    }
    //    tprintf::tprintf("malloc @ = @\n", sz, ptr);
    return ptr;
  }

  inline constexpr size_t getSize(void * ptr) {
    assert(RepoHeader<Size>::isValid());
    return RepoHeader<Size>::getBaseSize();
    //    return 0;
  }

  inline constexpr bool inBounds(void * ptr) {
    assert(RepoHeader<Size>::isValid());
    char * cptr = reinterpret_cast<char *>(ptr);
    return ((cptr >= &_buffer[0]) && (cptr <= &_buffer[(getNumberOfObjects()-1) * RepoHeader<Size>::getObjectSize()]));
  }
  
  inline void free(void * ptr) {
    assert(RepoHeader<Size>::isValid());
    assert(inBounds(ptr));
    assert(RepoHeader<Size>::getAllocated());
    memset(ptr, 'X', RepoHeader<Size>::getObjectSize()); // FIXME
    RepoHeader<Size>::decAllocated();
    assert(RepoHeader<Size>::getAllocated() <= getNumberOfObjects());
  }
    
protected:
  char _buffer[Size - sizeof(RepoHeader<Size>)];
};


#if 0
// A repo for a specific object size.
template <unsigned long ObjectSize, unsigned long Size>
//template <unsigned long Size>
class RepoX : public RepoBase<Size> {
public:
  enum { NumObjects = (Size - sizeof(RepoHeader)) / ObjectSize };
  
private:

  class Object {
  public:
    char buf[ObjectSize];
  };

public:
  
  RepoX() : RepoBase<Size>(ObjectSize)
  {
  }

  inline bool isFull() {
    return (RepoHeader::_allocated == NumObjects);
  }

  inline bool isEmpty() {
    return (RepoHeader::_allocated == 0);
  }

  inline void * malloc(size_t sz) {
    //    std::cout << "this = " << this << std::endl;
    assert (sz <= ObjectSize);
    if (RepoHeader::_allocated < NumObjects) {
      auto object = reinterpret_cast<Object *>(RepoBase<Size>::_buffer);
      auto * ptr = &object[RepoHeader::_allocated];
      assert(inBounds(ptr));
      RepoHeader::_allocated++;
      return ptr;
    } else {
      //      std::cout << "out of objects: _allocated = " << RepoHeader::_allocated << std::endl;
      return nullptr;
    }
  }

  inline constexpr size_t getSize(void * ptr) {
    return RepoHeader::getBaseSize();
    auto objPtr = reinterpret_cast<Object *>(ptr);
    if (inBounds(objPtr)) {
      return RepoBase<Size>::getBaseSize();
    }
    return 0;
  }

  inline constexpr bool inBounds(void * ptr) {
    auto objPtr = reinterpret_cast<Object *>(ptr);
    auto object = reinterpret_cast<Object *>(RepoBase<Size>::_buffer);
    return ((objPtr >= &object[0]) && (objPtr <= &object[NumObjects-1]));
  }
  
  inline void free(void * ptr) {
    assert(inBounds(ptr));
    RepoHeader::_allocated--;
    assert(RepoHeader::_allocated <= NumObjects);
  }
    
};
#endif

#endif