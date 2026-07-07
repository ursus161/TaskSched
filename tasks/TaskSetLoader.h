#pragma once
#include "Task.h"
#include <vector>
#include <memory>
#include <string>

// incarca un task set dintr-un fisier CSV (vezi configs/ pentru format).
// coloane: type,name,period,wcet,deadline,priority,release_time
// arunca std::runtime_error cu un mesaj clar (fisier:linie: motiv) la input invalid.
//
// duration e folosit doar pentru taskurile sporadice: le tratez worst-case,
// generand activari la fiecare MIT (perioada minima) pana la finalul simularii.
namespace TaskSetLoader {
    std::vector<std::unique_ptr<Task>> load(const std::string& path, int duration);
}
