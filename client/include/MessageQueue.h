#include <queue>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class MessageQueue
{
private:
    std::queue<json> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    void push(json &&js)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(js);
        cond_.notify_one();
    }
    json pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]
                   { return !queue_.empty(); });
        json js = queue_.front();
        queue_.pop();
        return js;
    }
    bool isEmpty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};