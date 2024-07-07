#pragma once
#include <stdint.h>
#include <map>
#include <vector>

/**
 * @brief The Buffer class
 * The buffer stores collections of IDs and messages, has a maximum capacity and provides two functions:
 * A setter which accepts the ID and the message of the ethernet packet and stores them.
 * A getter which accepts an ID and returns the corresponding message, if it exists. If the ID doesn't exist, it should
return a message of your choice.
 */
class Buffer
{
public:
    Buffer(uint32_t capacity)
        : capacity_(capacity)
    {
    }

    /**
     * @brief Return the message associated with the input ID.
     *
     * @param id The ID of the message
     * @returns the message associated with the input ID. Returns a vector of your choice if the ID doesn't exist.
     */
    std::vector<uint8_t> getElement(int id) {
        auto it = cache_map_.find(id);
        if (it != cache_map_.end()) {
            return it->second;
        }
        return {};
    }

    /**
     * @brief Store the message with the ID in the buffer
     *
     * @param id The ID of the message
     * @param data The message
     *
     * @note A setter can succeed only if there is enough space in the cache, namely the capacity is not full. If it's
     * full, then the first inserted element needs to be deleted from the cache before inserting the new one.
     */
    void setElement(int id, const std::vector<uint8_t>& data) {
        if (cache_map_.size() >= capacity_) {
            // Remove element: FIFO
            cache_map_.erase(cache_map_.begin());
        }
        cache_map_[id] = data;
    }

private:
    uint32_t capacity_{};
    std::map<int, std::vector<uint8_t>> cache_map_;
};