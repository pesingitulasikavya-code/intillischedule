#pragma once

#include "models.hpp"
#include <vector>

namespace intellischedule {

class ScheduleValidator {
public:
    static bool validate(ScheduleResult& result, int timeBudget = 0);
};

} // namespace intellischedule
