#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

template<class T>
class IKeeper {
 public:
  enum class KeeperResult
  {
    ERROR,
    OK,
  };

  /**
   * @brief Construct a new IKeeper object
   *
   * @param mem pointer to the array memory for holding items
   * @param capacity array capacity
   */
  IKeeper(T** mem, size_t capacity) : vect(mem), Max(capacity) {}

  /**
   * @brief Attempts to add new item to the next free container space
   *
   * @param c pointer to object to add to the container
   * @param allowduplicate if it is allowed to have more than one instance
   * of the object in container
   * @return KeeperResult: OK if the object was added to the container
   *                       ERROR if the object wasn't added
   */
  KeeperResult Add(T* c, bool allowduplicate = true) {
    size_t addedid = Max;

    // if count = Max the collection full, no one loop iteration will be run
    for (size_t i = 0; (i < Max) && (count < Max); i++) {
      if ((vect[i] == c) && (allowduplicate == false)) {
        // client already in collection, break loop and return fail
        break;
      }
      else if (vect[i] == nullptr) {
        // add new client
        vect[i] = c;
        count++;
        addedid = i;
        break;
      }
    }

    return (addedid == Max) ? (KeeperResult::ERROR) : (KeeperResult::OK);
  }

  /**
   * @brief Removes the object from the container
   *
   * @param c pointer to the object to be removed from the container
   * @return KeeperResult: OK object was removed from the container
   *                       ERROR object wasn't removed
   */
  KeeperResult Remove(T* c) {
    // For now removing prohibited
    return KeeperResult::ERROR;
  }

  /**
   * @brief Resets internal iterator index to zero (it is not thread safe API)
   *
   */
  void Start() {
    iter_id = 0;
  }

  /**
   * @brief Tests if the current index is on the last object of the container
   */
  bool Last() {
    return (iter_id >= count);
  }

  /**
   * @brief returns pointer to the object from container (container[internal_index])
   *
   * @return T* pointer to object or NULLPTR
   */
  T* Next() {
    T* ret = nullptr;

    if (iter_id < count) {
      ret = vect[iter_id];
      iter_id++;
    }

    assert(ret != nullptr);

    return ret;
  }

  /**
   * @brief Reads pointer to the object in the container with index @id
   *
   * @param id index of request object
   * @param refout reference on pointer for saving address of requested object
   * @return true if the object in the container is not nullptr
   */
  bool Item(size_t id, T*& refout) const {
    refout = id < Max ? vect[id] : nullptr;
    return ((refout != nullptr) ? (true) : (false));
  }


  /**
   * @brief Returns number of objects in the container
   */
  size_t Count() const {
    return count;
  }

  /**
   * @brief Returns internal container capacity
   */
  size_t Capacity() const {
    return Max;
  }

 private:
  T** const vect{nullptr};
  const size_t Max;
  size_t count{};
  size_t iter_id{};
};

template<typename C>
class AsKeeper : public IKeeper<C>, public C {
 protected:
  AsKeeper(C** array, size_t SIZE) : IKeeper<C>(array, SIZE) {}
};

template<typename T, size_t N>
class MemKeeper : public IKeeper<T> {
 public:
  MemKeeper() : IKeeper<T>(__raw__, N) {}

 private:
  T* __raw__[N] = {nullptr};
};
