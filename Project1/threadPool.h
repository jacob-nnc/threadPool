#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>
#include <future>

class threadPool
{
	std::vector<std::thread> thds;
	std::queue<std::function<void()>> tasks;
	bool stop;
	std::mutex mtx;
	std::condition_variable cond;

public:
	threadPool(int num)
	{
		stop = false;
		for (int i = 0; i < num; i++) thds.emplace_back([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> t(mtx);
					cond.wait(t, [this]() { return stop || !tasks.empty(); });
					if (stop && tasks.empty()) return;
					task = std::move(tasks.front());
					tasks.pop();
				}
				task();
			}
			});
	}

	~threadPool() {
		{
			std::unique_lock<std::mutex> t(mtx);
			stop = true;
		}
		cond.notify_all();
		for (auto& i : thds) {
			i.join();
		}
	}

	template <class F, class... Args>
	auto addTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		using return_type = decltype(f(args...));
		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(mtx);
			tasks.emplace([task]() { (*task)(); });
		}
		cond.notify_one();
		return res;
	}
};