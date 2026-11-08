#pragma once

#include "models.hpp"
#include <vector>

namespace intellischedule {

class IntervalScheduler {
public:
    static ScheduleResult schedule(const std::vector<Task>& tasks);
};

} // namespace intellischedule
