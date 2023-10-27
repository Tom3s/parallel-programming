#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <mutex>
#include <deque>
#include <condition_variable>

const std::string nPrefix = "-n";

std::vector<int> numbers1;
std::vector<int> numbers2;

std::condition_variable cv;

std::deque<int> productQueue;
std::mutex productMutex;

unsigned long long finalSum = 0;

bool isDone = false;

void producer() {
	for (int i = 0; i < numbers1.size(); i++) {
		std::lock_guard<std::mutex> lock(productMutex);

		productQueue.push_back(numbers1[i] * numbers2[i]);

		cv.notify_one();
	} 
	isDone = true;
	std::cout << "Producer done\n" << std::flush;
}

void consumer() {
	while (!isDone || !productQueue.empty()) {
		std::unique_lock<std::mutex> lock(productMutex);

		cv.wait(lock, []() { return !productQueue.empty(); });

		finalSum += productQueue.front();
		productQueue.pop_front();
	}

	std::string s = "Final sum: " + std::to_string(finalSum) + "\n";

	std::cout << s << std::flush;
}

int main(int nrArgs, char* args[]) {
	auto n = 100;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		}
	}

	for (int i = 0; i < n; i++){
		numbers1.push_back(i);
		numbers2.push_back(i);
	}


	std::thread producerThread(producer);
	std::thread consumerThread(consumer);

	producerThread.join();
	consumerThread.join();

}