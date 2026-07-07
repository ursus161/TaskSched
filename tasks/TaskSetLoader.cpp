#include "TaskSetLoader.h"
#include "PeriodicTask.h"
#include "AperiodicTask.h"
#include "SporadicTask.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <optional>

namespace {

// split pe virgula care pastreaza si campurile goale (inclusiv unul final)
std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> out;
    for (size_t start = 0; ; ) {
        size_t comma = line.find(',', start);
        out.push_back(line.substr(start, comma - start));
        if (comma == std::string::npos) return out;
        start = comma + 1;
    }
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

std::string lower(std::string s) {
    std::ranges::transform(s, s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::runtime_error err(const std::string& path, int line_no, const std::string& msg) {
    return std::runtime_error("TaskSetLoader: " + path + ":" + std::to_string(line_no) + ": " + msg);
}

// campul de la indexul idx: nullopt daca lipseste/gol/-1, arunca daca e text ne-numeric
std::optional<int> field(const std::vector<std::string>& f, size_t idx,
                         const std::string& col, const std::string& path, int line_no) {
    std::string s = idx < f.size() ? trim(f[idx]) : "";
    if (s.empty() || s == "-1") return std::nullopt;
    try { return std::stoi(s); }
    catch (...) { throw err(path, line_no, col + " nu e numar valid: '" + s + "'"); }
}

} // namespace

std::vector<std::unique_ptr<Task>> TaskSetLoader::load(const std::string& path, int duration) {
    std::ifstream fin(path);
    if (!fin.is_open())
        throw std::runtime_error("TaskSetLoader: nu pot deschide '" + path + "'");

    Task::resetIdCounter();  // idurile pornesc de la inceput la fiecare incarcare

    // la un throw in mijloc, unique_ptr-urile din vector se elibereaza singure
    std::vector<std::unique_ptr<Task>> tasks;
    std::string line;
    int line_no = 0;
    bool header_skipped = false;

    while (std::getline(fin, line)) {
        line_no++;
        if (trim(line).empty() || trim(line)[0] == '#') continue;  // goale / comentarii

        auto f = split(line);
        // prima linie utila care arata a header ("type,...") e sarita
        if (!header_skipped) {
            header_skipped = true;
            if (lower(trim(f[0])) == "type") continue;
        }

        // camp obligatoriu: arunca daca lipseste
        auto req = [&](size_t idx, const std::string& col) {
            if (auto v = field(f, idx, col, path, line_no)) return *v;
            throw err(path, line_no, col + " lipseste");
        };

        std::string type = lower(trim(f[0]));
        std::string name = f.size() > 1 ? trim(f[1]) : "";
        if (type.empty()) throw err(path, line_no, "coloana 'type' lipseste");
        if (name.empty()) throw err(path, line_no, "coloana 'name' lipseste");

        int wcet     = req(3, "wcet");
        int deadline = req(4, "deadline");
        int priority = field(f, 5, "priority", path, line_no).value_or(0);
        int release  = field(f, 6, "release_time", path, line_no).value_or(0);

        if (wcet <= 0)     throw err(path, line_no, "wcet trebuie sa fie > 0");
        if (deadline <= 0) throw err(path, line_no, "deadline trebuie sa fie > 0");
        if (release < 0)   throw err(path, line_no, "release_time nu poate fi negativ");

        if (type == "periodic") {
            int period = req(2, "period");
            if (period <= 0) throw err(path, line_no, "period trebuie sa fie > 0");
            tasks.push_back(std::make_unique<PeriodicTask>(name, priority, wcet, deadline, period, release));

        } else if (type == "aperiodic") {
            // pentru aperiodic period-ul e ignorat; release_time = momentul sosirii
            tasks.push_back(std::make_unique<AperiodicTask>(name, priority, wcet, deadline, release));

        } else if (type == "sporadic") {
            int mit = req(2, "period");  // period = min inter-arrival time
            if (mit <= 0) throw err(path, line_no, "period (MIT) trebuie sa fie > 0");
            // worst-case: activari la fiecare MIT, din release pana la finalul simularii
            std::vector<int> triggers;
            for (int t = release; t < duration; t += mit) triggers.push_back(t);
            tasks.push_back(std::make_unique<SporadicTask>(name, priority, wcet, deadline, mit, triggers));

        } else {
            throw err(path, line_no, "type necunoscut: '" + type + "' (periodic|aperiodic|sporadic)");
        }
    }

    if (tasks.empty())
        throw std::runtime_error("TaskSetLoader: niciun task valid in '" + path + "'");

    return tasks;
}
