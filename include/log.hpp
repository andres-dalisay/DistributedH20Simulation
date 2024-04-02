struct Log {
    std::string id;
    std::string type;
};

struct timestamp {
    std::string getCurrentTime() {
        // Get the current timestamp
        auto now = std::chrono::system_clock::now();

        // Convert to time_t
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // Convert to local time using localtime_s
        std::tm local_time;
        localtime_s(&local_time, &now_c);

        // Format the local time
        std::ostringstream oss;
        oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};