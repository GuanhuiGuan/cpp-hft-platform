#pragma once

#include "../spsc_lfq.h"
#include "../thread_util.h"

#include <fstream>

namespace infra {

    constexpr size_t BUFFER_CAP = 1024 * 1024 * 8;

    enum class LogType : int8_t {
        INFO,
        WARN,
        ERROR
    };
    const static std::string LOG_TYPE_NAMES[] {"INFO", "WARN", "ERROR"};

    namespace {
        enum class EntityType : int8_t {
            CHAR,
            INT,
            LONG,
            LL,
            UINT,
            UL,
            ULL,
            FLOAT,
            DOUBLE,
        };

        struct Entity {
            EntityType type_;
            union {
                char c_;
                int i_;
                long l_;
                long long ll_;
                unsigned int ui_;
                unsigned long ul_;
                unsigned long long ull_;
                float f_;
                double d_;
            } u_;
            Entity() = default;
            Entity(const char& x) {type_ = EntityType::CHAR; u_.c_ = x;}
            Entity(const int& x) {type_ = EntityType::INT; u_.i_ = x;}
            Entity(const long& x) {type_ = EntityType::LONG; u_.l_ = x;}
            Entity(const long long& x) {type_ = EntityType::LL; u_.ll_ = x;}
            Entity(const unsigned int& x) {type_ = EntityType::UINT; u_.ui_ = x;}
            Entity(const unsigned long& x) {type_ = EntityType::UL; u_.ul_ = x;}
            Entity(const unsigned long long& x) {type_ = EntityType::ULL; u_.ull_ = x;}
            Entity(const float& x) {type_ = EntityType::FLOAT; u_.f_ = x;}
            Entity(const double& x) {type_ = EntityType::DOUBLE; u_.d_ = x;}
        };
    }

    template<typename L>
    auto funcFlushBuffer(L& logger) {
        Entity entity;
        while (logger.running()) {
            logger.flushBufferOnce(entity);
            std::this_thread::sleep_for(10ms);
        }
    }

    class Logger {
    public:
        void flushBufferOnce(Entity& entity) {
            for (size_t i = 0; i < buffer_.size(); ++i) {
                if (buffer_.dequeue(entity)) {
                    write(entity);
                }
            }
            file_.flush();
        }
        void flushBuffer() {
            Entity entity;
            while (running_) {
                flushBufferOnce(entity);
                std::this_thread::sleep_for(10ms);
            }
        }

        explicit Logger(const std::string& filename) : filename_{filename}, buffer_{BUFFER_CAP}, running_{true} {
            file_.open(filename);
            ASSERT(file_.is_open(), "Failed to open file " + filename);
            consumer_ = startThread(-1, filename, [this]() { flushBuffer(); });
            // consumer_ = startThread(-1, filename, funcFlushBuffer<Logger>, *this);
            ASSERT(consumer_ != nullptr, "Failed to start log consumer thread");
            std::cerr << "Started logger " << filename << std::endl;
        }
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;
        ~Logger() noexcept {
            stop();
        }

        template<typename... Args>
        void log(LogType type, const char* fmt, const Args&... args) {
            auto now = std::chrono::system_clock::now();
            enqueue(LOG_TYPE_NAMES[(int8_t)type]);
            enqueue(' ');
            enqueue(now.time_since_epoch().count());
            enqueue(' ');
            internalLog(fmt, args...);
            enqueue('\n');
        }

        bool running() const {return running_;}

        void stop() {
            std::cerr << "Terminating and flushing logger " << filename_ << std::endl;
            if (running_) {
                while (!buffer_.empty()) {
                std::this_thread::sleep_for(100ms);
                }
                running_ = false;
                consumer_->join();
                file_.close();
            }
            std::cerr << "Logger " << filename_ << " terminated" << std::endl;
        }
        
    private:
        const std::string filename_;
        SpscQueue<Entity> buffer_;
        volatile bool running_;
        std::ofstream file_;
        std::unique_ptr<std::thread> consumer_;

        void write(const Entity& entity) {
            switch (entity.type_) {
                case EntityType::CHAR:
                    file_ << entity.u_.c_; break;
                case EntityType::INT:
                    file_ << entity.u_.i_; break;
                case EntityType::LONG:
                    file_ << entity.u_.l_; break;
                case EntityType::LL:
                    file_ << entity.u_.ll_; break;
                case EntityType::UINT:
                    file_ << entity.u_.ui_; break;
                case EntityType::UL:
                    file_ << entity.u_.ul_; break;
                case EntityType::ULL:
                    file_ << entity.u_.ull_; break;
                case EntityType::FLOAT:
                    file_ << entity.u_.f_; break;
                case EntityType::DOUBLE:
                    file_ << entity.u_.d_; break;
            }
        }

        template<typename T>
        void enqueue(const T& x) {
            buffer_.enqueue(Entity(x));
        }
        void enqueue(const char* x) {
            while (*x) {
                enqueue(*x++);
            }
        }
        void enqueue(const std::string& x) {
            enqueue(x.c_str());
        }

        template<typename T, typename... Args>
        void internalLog(const char* fmt, const T& val, const Args&... args) {
            while (*fmt) {
                if (*(fmt) == '%') {
                    [[unlikely]] if (*(fmt+1) == '%') ++fmt; // %% -> %
                    else {
                        enqueue(val);
                        internalLog(fmt + 1, args...);
                        return;
                    }
                }
                enqueue(*fmt++);
            }
        }
        void internalLog(const char* fmt) {
            while (*fmt) {
                if (*(fmt) == '%') {
                    [[unlikely]] if (*(fmt+1) == '%') ++fmt; // %% -> %
                }
                enqueue(*fmt++);
            }
        }
    };

}