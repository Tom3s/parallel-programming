#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <mutex>
#include <chrono>
using namespace std::chrono;
#define clock high_resolution_clock::now

#include "BS_thread_pool_light.hpp"

typedef std::vector<std::vector<int>> Matrix;

const std::string nPrefix = "-n";
const std::string mPrefix = "-m";

Matrix a;
Matrix b;

Matrix result;

void calculateProduct(int i, int j) {
	auto sum = 0;
	for (int k = 0; k < a[0].size(); k++) {
		sum += a[i][k] * b[k][j];
	}
	result[i][j] = sum;
}

void startTask(int iStart, int iEnd) {
	for (int i = iStart; i < iEnd; i++) {
		if (i >= b.size()) {
			break;
		}
		for (int j = 0; j < b[0].size(); j++) {
			calculateProduct(i, j);
		}
	}
}

int main(int nrArgs, char* args[]) {
	auto n = 5;
	auto m = 3;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		} else if (args[i] == mPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -m (must be a number)");
			}
			m = std::stoi(args[i + 1]);
		}
	}

	a = Matrix(n, std::vector<int>(m, 1));
	b = Matrix(m, std::vector<int>(n, 1));

	for (auto& row : a) {
		for (auto& element : row) {
			element = rand() % 10 + 1;
		}
	}

	for (auto& row : b) {
		for (auto& element : row) {
			element = rand() % 10 + 1;
		}
	}

	result = Matrix(n, std::vector<int>(n, 0));

	auto startTime = clock();

	auto threads = std::vector<std::thread>();

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			threads.emplace_back(calculateProduct, i, j);
		}
	}

	for (auto& thread : threads) {
		thread.join();
	}

	auto endTime = clock();

	int ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Time to calculate product for " << n << "x" << m << " matrix:" << ms << "ms" << std::endl;

	// std::cout << "Matrix A: " << std::endl;
	// for (auto& row : a) {
	// 	for (auto& element : row) {
	// 		std::cout << element << " ";
	// 	}
	// 	std::cout << std::endl;
	// }

	// std::cout << "Matrix B: " << std::endl;
	// for (auto& row : b) {
	// 	for (auto& element : row) {
	// 		std::cout << element << " ";
	// 	}
	// 	std::cout << std::endl;
	// }

	// for (auto& row : result) {
	// 	for (auto& element : row) {
	// 		std::cout << element << " ";
	// 	}
	// 	std::cout << std::endl;
	// }

	result = Matrix(n, std::vector<int>(n, 0));

	startTime = clock();

	threads = std::vector<std::thread>();

	for (int i = 0; i < n; i+=2) {
		threads.emplace_back(startTask, i, i + 2);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Time to calculate product for " << n << "x" << m << " matrix with batching: `" << ms << " ms`" << std::endl;

	result = Matrix(n, std::vector<int>(n, 0));

	startTime = clock();

	threads = std::vector<std::thread>();

	BS::thread_pool_light pool(std::thread::hardware_concurrency() - 1);

	for (int i = 0; i < n; i+=2) {
		pool.push_task(startTask, i, i + 2);
	}

	pool.wait_for_tasks();

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "- Time to calculate product for " << n << "x" << m << " matrix with pool: `" << ms << " ms`" << std::endl;
}