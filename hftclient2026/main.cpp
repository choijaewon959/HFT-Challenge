#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "Profiler.h"
#include "Parser.h"
#include "mm.h"

using namespace std;

#define PROFILE_OUTPUT "profile.csv"
#define MODEL_VERSION "matmul_direct_trace_openmp_on"

int main(int argc, char** argv) {
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <host> <port> <team_name>\n";
        return 1;
    }

    string host = argv[1];
    int port = stoi(argv[2]);
    string team = argv[3];

    // --- Connect to server ---
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid host address: " << host << endl;
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect failed");
        close(sock);
        return 1;
    }

    cout << "Connected to server at " << host << ":" << port << endl;

    // Send team name
    string intro = team + "\n";
    send(sock, intro.c_str(), intro.size(), 0);

    cout << "Sent team name: " << team << endl;
    cout << "Waiting for challenges..." << endl;

    // Use your fast socket parser
    FastParser reader(sock);

    Profiler profiler(PROFILE_OUTPUT, MODEL_VERSION);

    while (true) {
        int challengeId;
        int N;

        if (!reader.readInt(challengeId)) {
            cout << "Disconnected or failed reading challengeId." << endl;
            break;
        }

        if (!reader.readInt(N)) {
            cout << "Disconnected or failed reading N." << endl;
            break;
        }

        auto t1 = profiler.now();

        if (N != 128) {
            cerr << "Bad N received: " << N << ". Stream may be corrupted." << endl;
            break;
        }

        cout << "Received challenge " << challengeId << " with N = " << N << endl;

        vector<int> A(N * N);
        vector<int> B(N * N);

        for (int i = 0; i < N * N; ++i) {
            if (!reader.readInt(A[i])) {
                cerr << "Failed reading A" << endl;
                close(sock);
                return 1;
            }
        }

        auto t2 = profiler.now();

        for (int i = 0; i < N * N; ++i) {
            if (!reader.readInt(B[i])) {
                cerr << "Failed reading B" << endl;
                close(sock);
                return 1;
            }
        }

        auto t3 = profiler.now();

        // Calculate tr(AB)
        mm compute;

        // Choose one:
        // long long answer = compute.trace_via_matmul(A, B);
        // long long answer = compute.trace_direct(A, B);
        long long answer = compute.trace_direct_openmp(A, B);

        auto t4 = profiler.now();

        string answerStr = to_string(challengeId) + " " + to_string(answer) + "\n";

        send(sock, answerStr.c_str(), answerStr.size(), 0);

        auto t5 = profiler.now();

        long long read_A_us = profiler.usBetween(t1, t2);
        long long read_B_us = profiler.usBetween(t2, t3);
        long long compute_us = profiler.usBetween(t3, t4);
        long long send_us = profiler.usBetween(t4, t5);
        long long total_us = profiler.usBetween(t1, t5);

        profiler.log(
            challengeId,
            N,
            read_A_us,
            read_B_us,
            compute_us,
            send_us,
            total_us,
            answer
        );

        cout << "Sent answer: " << answer << endl;
    }

    close(sock);
    return 0;
}