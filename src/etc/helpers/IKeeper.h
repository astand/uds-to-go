#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "static-allocator.h"

/// @brief Type pointer redefinition
/// @tparam T target type
template<class T>
using Tptr_t = T*;

/// @brief Class container to keep array of pointers to T
/// @tparam T Type of keeping elements
template<class T>
class IKeeper {

 public:
  /// @brief Private container pointer-type redifinition
  using ElemPtr_t = Tptr_t<T>;

  /// @brief Result of any keeper opearion
  enum class KeeperResult {
    ERROR,
    OK,
  };

  /// @brief Constructor
  /// @param mem Pointer to array in memory allocated for keeping elements
  /// @param capacity Number of elements in the element array
  IKeeper(ElemPtr_t mem[], size_t capacity) : elemArray(mem), ElemMaxNumber(capacity) {}

  /// @brief Adds new element to the next empty position in the array
  /// @param elem Pointer to the instance to be adde to the container
  /// @param multi Flag to permit adding more than one pointer on the same element
  /// @return Result of adding operation
  KeeperResult Add(ElemPtr_t elem, bool multi = true) {
    size_t addedPosition = ElemMaxNumber;

    for (size_t i = 0; (i < ElemMaxNumber) && (currElemNumber < ElemMaxNumber); i++) {
      if ((elemArray[i] == elem) && (multi == false)) {
        // element is already in collection, break the loop and return error
        break;
      } else if (elemArray[i] == nullptr) {
        // add new element
        elemArray[i] = elem;
        currElemNumber++;
        addedPosition = i;
        break;
      }
    }

    return (addedPosition == ElemMaxNumber) ? (KeeperResult::ERROR) : (KeeperResult::OK);
  }

  /// @brief Removes requested element from the container
  /// @param elemToRemove Pointer to the element to remove
  /// @return Result of the remove operation
  KeeperResult Remove(ElemPtr_t elemToRemove) {
    // Removing is not allowed yet
    return KeeperResult::ERROR;
  }

  /// @brief Resets internal iteration index
  void StartIteration() {
    iterId = 0;
  }

  /// @brief Checks iteration status
  /// @return true if the current iteration index is the last one, otherwise false
  bool IsLastIteration() {
    return (iterId >= currElemNumber);
  }

  /// @brief Shifts iteration poisition to the next element
  /// @return Pointer to the current iteration element
  ElemPtr_t IterNextElem() {
    ElemPtr_t ret = nullptr;

    if (iterId < currElemNumber) {
      ret = elemArray[iterId];
      iterId++;
    }

    assert(ret != nullptr);

    return ret;
  }

  /// @brief Tries to read element address from the container
  /// @param id Element index
  /// @param refout Reference to the pointer on element type
  /// @return true if the @refout is set to some element, false when it is set to nullptr
  bool TryReadElem(size_t id, ElemPtr_t& refout) const {
    refout = ((id < ElemMaxNumber) ? elemArray[id] : nullptr);
    return (refout != nullptr) ? true : false;
  }

  /// @brief Gets current number of the elements in the container
  /// @return Number of elements
  size_t Count() const {
    return currElemNumber;
  }

  /// @brief Gets maximum capacity of the container
  /// @return Container capacity in number of elements
  size_t Capacity() const {
    return ElemMaxNumber;
  }

 private:
  ElemPtr_t* const elemArray{nullptr};
  const size_t ElemMaxNumber;
  size_t currElemNumber{};
  size_t iterId{};
};

/// @brief Template class which combines IKeeper<C> and C functionality itself
/// @details The AsKeeper is a proxy template class for the cases when
/// the type shall provide some interface but needs to keep multiple
/// instances of the type C and perform interfaces calls to these instances
/// @tparam C Type which must be wrapped by AsKeeper
template<typename C>
class AsKeeper : public IKeeper<C>, public C {

 protected:
  /// @brief Constructor
  /// @param array Pointer to the array allocated for this container memory
  /// @param capacity Number of elements in the allocated array
  AsKeeper(C** array, size_t capacity) : IKeeper<C>(array, capacity) {}
};

/// @brief IKeeper extension with internal memory allocation
/// @tparam T Container element type
/// @tparam N Container array capacity
template<typename T, size_t N>
class MemKeeper : public IKeeper<T> {

 public:
  /// @brief Constructor
  /// @tparam T Type of container element
  /// @tparam N Container capacity
  MemKeeper() : IKeeper<T>(reinterpret_cast<Tptr_t<T>*>(statAllocator.ptr()), N) {}

 private:
  /// @brief Memory for the container
  StaticMemAllocator<Tptr_t<T>, N> statAllocator;
};

/// @brief AsKeeper extension with internal memory allocation
/// @tparam T Type of container element
/// @tparam N Container capacity
template<typename T, size_t N>
class MemAsKeeper : public AsKeeper<T> {

 public:
  /// @brief Constructor
  /// @tparam T Type of container element
  /// @tparam N Container capacity
  MemAsKeeper() : AsKeeper<T>(reinterpret_cast<Tptr_t<T>*>(statAllocator.ptr()), N) {}

 private:
  /// @brief Memory for the container
  StaticMemAllocator<Tptr_t<T>, N> statAllocator;
};
