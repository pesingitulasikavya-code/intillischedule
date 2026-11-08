#pragma once

#include "models.hpp"
#include <vector>

namespace intellischedule {

class KnapsackGreedyScheduler {
public:
    static ScheduleResult schedule(const std::vector<Task>& tasks, int timeBudget);
};

} // namespace intellischedule
