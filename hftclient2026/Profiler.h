#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;

class Profiler {
private:
    string filename;
    string trialName;

public:
    using Clock = high_resolution_clock;
    using TimePoint = high_resolution_clock::time_point;

    Profiler(const string& csvFile, const string& trial)
        : filename(csvFile), trialName(trial) {
        writeHeaderIfNeeded();
    }

    TimePoint now() const {
        return Clock::now();
    }

    long long usBetween(const TimePoint& start, const TimePoint& end) const {
        return duration_cast<microseconds>(end - start).count();
    }

    void writeHeaderIfNeeded() {
        ifstream check(filename);
        bool existsAndNonEmpty =
            check.good() && check.peek() != ifstream::traits_type::eof();
        check.close();

        if (!existsAndNonEmpty) {
            ofstream out(filename, ios::app);

            if (!out.is_open()) {
                cerr << "Failed to open CSV file for header: " << filename << endl;
                return;
            }

            out << "trial_name,"
                << "challenge_id,"
                << "N,"
                // << "header_us,"
                << "read_A_us,"
                << "read_B_us,"
                << "compute_us,"
                << "send_us,"
                << "total_us,"
                << "answer\n";
        }
    }

    void log(
        int challengeId,
        int N,
        // long long header_us,
        long long read_A_us,
        long long read_B_us,
        long long compute_us,
        long long send_us,
        long long total_us,
        long long answer
    ) {
        ofstream out(filename, ios::app);

        if (!out.is_open()) {
            cerr << "Failed to open CSV file for logging: " << filename << endl;
            return;
        }

        out << trialName << ","
            << challengeId << ","
            << N << ","
            // << header_us << ","
            << read_A_us << ","
            << read_B_us << ","
            << compute_us << ","
            << send_us << ","
            << total_us << ","
            << answer << "\n";
    }
};

#endif