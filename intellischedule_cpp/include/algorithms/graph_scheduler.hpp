#pragma once

#include "models.hpp"
#include <vector>

namespace intellischedule {

class GraphScheduler {
public:
    static ScheduleResult schedule(const std::vector<Task>& tasks, int horizonStart = 0);
};

} // namespace intellischedule
