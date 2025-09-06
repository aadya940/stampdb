#include <vector>
#include <algorithm>

#include "../include/internal/csvparse.hpp"


// Insert an index while maintaining the FullIndex sorted by time
void insertIndexSorted(FullIndex& fullIndex, const Index& newIndex) {
    auto& indices = fullIndex.indices;
    auto it = std::lower_bound(indices.begin(), indices.end(), newIndex,
        [](const Index& a, const Index& b) {
            return a.time < b.time;
        });
    indices.insert(it, newIndex);
    
    // Update MAX_ROWNUM if needed
    if (newIndex.index > fullIndex.MAX_ROWNUM) {
        fullIndex.MAX_ROWNUM = newIndex.index;
    }
}


// Find the first index with time >= targetTime
// Returns an iterator to the first element not less than targetTime
std::vector<Index>::const_iterator findFirstAfterOrEqualTime(
    const FullIndex& fullIndex, double targetTime) {
    const auto& indices = fullIndex.indices;
    return std::lower_bound(indices.begin(), indices.end(), targetTime,
        [](const Index& a, double time) {
            return a.time < time;
        });
}


// Find all indices within a time range [startTime, endTime]
std::vector<Index> findInTimeRange(const FullIndex& fullIndex, 
                                 double startTime, double endTime) {
    const auto& indices = fullIndex.indices;
    auto startIt = findFirstAfterOrEqualTime(fullIndex, startTime);
    
    auto endIt = std::upper_bound(startIt, indices.end(), endTime,
        [](double time, const Index& b) {
            return time < b.time;
        });
    
    return {startIt, endIt};
}


// Get the next available row number
int getNextRowNumber(const FullIndex& fullIndex) {
    return fullIndex.MAX_ROWNUM + 1;
}
