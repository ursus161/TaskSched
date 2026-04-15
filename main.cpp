#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "scheduler/stats/Stats.h"
#include <iostream>

int main() {
    // scenariu: simulare de 20 tickuri cu 3 taskuri
    // sensor: periodic, perioada 5, wcet 2 -> joburi la t=0,5,10,15
    // display: periodic, perioada 10, wcet 3 -> joburi la t=0,10
    // button: aperiodic la t=7, wcet 2

    Stats stats;
    stats.registerTask(1, "sensor", "Periodic");
    stats.registerTask(2, "display", "Periodic");
    stats.registerTask(3, "button", "Aperiodic");

    // t=0: sensor si display release-uite, sensor ruleaza (prioritate)
    stats.onRelease(1);
    stats.onRelease(2);
    stats.onTick(true);   // t=0 sensor ruleaza
    stats.onTick(true);   // t=1 sensor ruleaza
    // sensor termina jobul 1, response time = 2
    stats.onComplete(1, 2);

    // t=2-4: display ruleaza 3 tickuri, termina
    stats.onTick(true);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(2, 5);  // a inceput la t=0, termina la t=5

    // t=5: sensor release job 2
    stats.onRelease(1);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(1, 2);

    // t=7: button release, preempteaza nimic (cpu era pe sensor care a terminat)
    stats.onRelease(3);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(3, 2);

    // t=9: idle un tick
    stats.onTick(false);

    // t=10: sensor si display iar
    stats.onRelease(1);
    stats.onRelease(2);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(1, 2);
    // display preemptat de sensor la release-ul urmator
    stats.onTick(true);
    stats.onPreempt(2);
    stats.onRelease(1);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(1, 2);

    // display nu mai apuca sa termine inainte de deadline
    stats.onDeadlineMiss(2);
    stats.onTick(true);
    stats.onTick(true);
    stats.onComplete(2, 10);

    std::cout << stats;
    return 0;
}