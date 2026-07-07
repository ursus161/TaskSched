CXX = g++
CXXFLAGS = -std=c++23 -Wall -g \
	-Itasks -Ischeduler -Ischeduler/stats \
	-Ischeduler/queue -Ischeduler/policies -Ischeduler/trace \
	-pthread

SRCS = tasks/Task.cpp tasks/PeriodicTask.cpp tasks/AperiodicTask.cpp \
       tasks/SporadicTask.cpp tasks/TaskSetLoader.cpp \
       scheduler/Scheduler.cpp \
       scheduler/queue/HeapReadyQueue.cpp \
       scheduler/stats/EventQueue.cpp \
       scheduler/trace/CsvTraceSink.cpp \
       scheduler/stats/TaskStats.cpp scheduler/stats/Stats.cpp \
       main.cpp

tasksched:
	$(CXX) $(CXXFLAGS) -o tasksched $(SRCS)

clean:
	rm -f tasksched

run:
	make tasksched
	./tasksched

.PHONY: clean run
