#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

template<class T>
class IKeeper {
 public:
  enum KeeperResult
  {
    ERROR,
    OK,
  };

  /** --------------------------------------------------------------------------
   * @brief construction with direct passing resources for collection
   * */
  IKeeper(T** mem, size_t capacity) : vect(mem), Max(capacity) {}

  /** ------------------------------------------------------------------------
   * @brief adding new instance to collection
   * @param c pointer to instance
   * @param dub allowing or prohibing dublicates on adding
   * @retval result of attemt
   * */
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

    return (addedid == Max) ? (ERROR) : (OK);
  }

  /** --------------------------------------------------------------------------
   * @brief removing instance from collection.
   * @param c pointer to instance to be deleted
   * @retval result of removing operation
   * */
  KeeperResult Remove(T* c) {
    // This method must perform two parts:
    // 1 find client and clear its position
    // 2 remove empty hole by putting last pointer from the tail to
    // the cell of previously deleted client
    // 3 decrement count
    // For now removing prohibited
    return ERROR;
  }

  /** --------------------------------------------------------------------------
   * @brief resets private cursor to the beginning of collection
   * ATTENTION - this API is not thread safe (!!!)
   * */
  void Start() {
    iter_id = 0;
  }


  /** --------------------------------------------------------------------------
   * @brief checks if the last Next() call returned last item from collection
   * ATTENTION - this API is not thread safe (!!!)
   * @retval true if next Next() call will return nullptr
   * */
  bool Last() {
    return (iter_id >= count);
  }

  /** --------------------------------------------------------------------------
   * @brief returns pointer to item from collection if the current position
   * ATTENTION - this API is not thread safe (!!!)
   * of the cursor is inside the range
   * */
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
   * @brief reads instance from keeper position
   *
   * @param id position number
   * @param refout reference to user scoped pointer var
   * @return true @refout points to instance
   * @return false @refout points to nullptr
   */
  bool Item(size_t id, T*& refout) const {
    refout = id < Max ? vect[id] : nullptr;
    return ((refout != nullptr) ? (true) : (false));
  }

  /** --------------------------------------------------------------------------
   * @retval number of previously added items to the collection
   * */
  size_t Count() const {
    return count;
  }

  size_t Capacity() const {
    return Max;
  }

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

 private:
  T** const vect{nullptr};
  const size_t Max;
  size_t count{};
  size_t iter_id{};
};


template<typename T, size_t N>
class MemKeeper : public IKeeper<T> {
 public:
  MemKeeper() : IKeeper<T>(__raw__, N) {}

 private:
  T* __raw__[N] = {0};
};
