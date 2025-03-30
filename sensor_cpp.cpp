#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>

#ifdef _WIN32
#include <windows.h>

// Define CLOCK_MONOTONIC for Windows
#define CLOCK_MONOTONIC 1

// Implement clock_gettime for Windows
int clock_gettime(int, struct timespec *tp) {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);

    tp->tv_sec = counter.QuadPart / frequency.QuadPart;
    tp->tv_nsec = (counter.QuadPart % frequency.QuadPart) * 1000000000 / frequency.QuadPart;
    return 0;
}
#endif

class SensorMonitor {
public:
    SensorMonitor() {
        messages = {"Low", "High", "Error occurred"};
    }

    void checkTimingFailure(int max_time_us) {
        struct timespec start{}, end{};
        clock_gettime(CLOCK_MONOTONIC, &start);

        int sensorValue;
        int status = STATUS_OK;
        int count = 0;

        while (count < MAX_NUMBER_OF_SAMPLES) {
            status = readSensor(sensorValue);
            if (status == STATUS_STOPPED) {
                break;
            } else if (status == STATUS_FAILED) {
                reportSensorFailure();
                return;
            }
            handleSensorValue(sensorValue);
            count++;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        long elapsed_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

        // Print timing result
        std::cout << "\n--------------------------------------------------\n";
        if (elapsed_time_us > max_time_us) {
            std::cout << "🚨 Timing failure: Execution took " << elapsed_time_us << " µs (exceeded " << max_time_us << " µs)\n";
        } else {
            std::cout << "✅ Timing success: Execution took " << elapsed_time_us << " µs (within " << max_time_us << " µs)\n";
        }
        std::cout << "--------------------------------------------------\n";
    }

private:
    static constexpr int STATUS_OK = 0;
    static constexpr int STATUS_FAILED = 1;
    static constexpr int STATUS_STOPPED = 2;
    static constexpr int MAX_NUMBER_OF_SAMPLES = 33;//changed samples numbers

    enum MessageType { VALUE_LOW_MSG, VALUE_HIGH_MSG, ERROR_MSG };

    std::vector<std::string> messages;

    int readSensor(int &value) {
        static int v = 0;
        value = v++;
        return (v > MAX_NUMBER_OF_SAMPLES) ? STATUS_STOPPED : STATUS_OK;
    }

    void printMessage(MessageType msgIndex, int value) {
        if (msgIndex < 0 || msgIndex > ERROR_MSG) {
            std::cout << "Value: " << value << ", State: Undefined\n";
        } else {
            std::cout << "Value: " << value << ", State: " << messages[msgIndex] << "\n";
        }
    }

    void reportSensorFailure() {
        printMessage(ERROR_MSG, 0);
    }

    void handleSensorValue(int value) {
        MessageType index = ERROR_MSG;
        if (value >= 0 && value <= 10) {
            index = VALUE_LOW_MSG;
        } else if (value > 10 && value <= 20) {
            index = VALUE_HIGH_MSG;
        }
        printMessage(index, value);
    }
};

int main() {
    SensorMonitor monitor;
    char choice;

    do {
        int max_time_us;
        std::cout << "\n==================================================\n";
        std::cout << "🕒 Please enter the expected run time in microseconds (µs): ";

        while (!(std::cin >> max_time_us)) {
            std::cout << "❌ Invalid input! Please enter a valid number: ";
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard input
        }

        monitor.checkTimingFailure(max_time_us);

        std::cout << "\nWould you like another run? (y to continue, q to quit): ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

    } while (choice == 'y' || choice == 'Y');

    std::cout << "\n🚪 Exiting program. Goodbye!\n";
    return 0;
}
