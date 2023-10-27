#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <mutex>

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

	std::cout << "Matrix A: " << std::endl;
	for (auto& row : a) {
		for (auto& element : row) {
			element = rand() % 10 + 1;
			std::cout << element << " ";
		}
		std::cout << std::endl;
	}

	std::cout << "Matrix B: " << std::endl;
	for (auto& row : b) {
		for (auto& element : row) {
			element = rand() % 10 + 1;
			std::cout << element << " ";
		}
		std::cout << std::endl;
	}

	result = Matrix(n, std::vector<int>(n, 0));

	auto threads = std::vector<std::thread>();

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			threads.emplace_back(calculateProduct, i, j);
		}
	}

	for (auto& thread : threads) {
		thread.join();
	}

	for (auto& row : result) {
		for (auto& element : row) {
			std::cout << element << " ";
		}
		std::cout << std::endl;
	}
}