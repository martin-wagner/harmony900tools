// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <stdexcept>
#include <unordered_set>

namespace lib
{

/**
 * @brief Singleton that generates unique uint32_t UIDs.
 *
 * UIDs are issued sequentially starting from a configurable start value.
 * A set of reserved/already-used UIDs can be registered to prevent reuse.
 */
class UidGenerator
{
  public:
    /**
     * @brief Returns the singleton instance.
     * @note Call initialize() before the first getInstance() if you need
     *       a non-default start value or a reserved set.
     */
    static UidGenerator& getInstance()
    {
      static UidGenerator instance;
      return instance;
    }

    /**
     * @brief (Re-)initializes the generator.
     * @param startValue  First UID that may be issued.
     * @param reserved    UIDs that must never be issued.
     *
     * Safe to call before the first getInstance(); calling it after UIDs
     * have already been issued resets the counter and clears used UIDs
     * (except the reserved set you pass in).
     */
    static void initialize(uint32_t startValue = 1,
        std::unordered_set<uint32_t> reserved = { })
    {
      UidGenerator &gen = getInstance();
      gen.nextId = startValue;
      gen.usedIds = std::move(reserved);
    }

    /**
     * @brief Generates the next available UID.
     * @return A UID that has not been issued or reserved before.
     * @throws std::overflow_error if all uint32_t values are exhausted.
     */
    uint32_t generate()
    {
      while (true) {
        if (nextId == 0) {
          // wrapped around — everything is exhausted
          throw std::overflow_error(
              "UidGenerator: all uint32_t UIDs exhausted");
        }

        uint32_t candidate = nextId++;

        if (usedIds.find(candidate) == usedIds.end()) {
          usedIds.insert(candidate);
          return candidate;
        }
      }
    }

    /**
     * @brief Marks a UID as used so it will never be issued by generate().
     * @param uid  UID to reserve.
     */
    void markUsed(uint32_t uid)
    {
      usedIds.insert(uid);
    }

    /** @brief Returns true if the UID has already been issued or reserved. */
    bool isUsed(uint32_t uid) const
    {
      return usedIds.find(uid) != usedIds.end();
    }

    // Non-copyable, non-movable
    UidGenerator(const UidGenerator&) = delete;
    UidGenerator& operator=(const UidGenerator&) = delete;
    UidGenerator(UidGenerator&&) = delete;
    UidGenerator& operator=(UidGenerator&&) = delete;

  private:
    UidGenerator() :
        nextId(1)
    {
    }

    uint32_t nextId;
    std::unordered_set<uint32_t> usedIds;
};

}
