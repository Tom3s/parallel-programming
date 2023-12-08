#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <mutex>
#include <chrono>

using namespace std::chrono;
#define clock high_resolution_clock::now

std::vector<int> parallelMultiplyPolynomials(const std::vector<int>& poly1, const std::vector<int>& poly2, int num_threads);


std::vector<int> multiplyPolynomials(const std::vector<int>& poly1, const std::vector<int>& poly2, int start, int end) {
    int n = poly1.size();
    int m = poly2.size();
    std::vector<int> result(n + m - 1, 0);

    for (int i = start; i < end; ++i) {
        for (int j = 0; j < m; ++j) {
            result[i + j] += poly1[i] * poly2[j];
        }
    }

    return result;
}

std::vector<int> parallelKaratsuba(const std::vector<int>& poly1, const std::vector<int>& poly2) {
    int n = poly1.size();

    // Base case
    if (n <= 16) {
        return multiplyPolynomials(poly1, poly2, 0, n);
    }

    int mid = n / 2;

    // Divide polynomials into two halves
    std::vector<int> a(poly1.begin(), poly1.begin() + mid);
    std::vector<int> b(poly1.begin() + mid, poly1.end());
    std::vector<int> c(poly2.begin(), poly2.begin() + mid);
    std::vector<int> d(poly2.begin() + mid, poly2.end());

    // Recursive steps in parallel
    std::vector<int> ac, bd, ad_bc;
	auto a_plus_b = std::vector<int>(mid);
	auto c_plus_d = std::vector<int>(mid);

    std::vector<std::thread> threads;

    threads.emplace_back([&]() { ac = parallelKaratsuba(a, c); });
    threads.emplace_back([&]() { bd = parallelKaratsuba(b, d); });

    for (int i = 0; i < mid; ++i) {
        a_plus_b[i] = a[i] + b[i];
		c_plus_d[i] = c[i] + d[i];
    }

    threads.emplace_back([&]() { ad_bc = parallelKaratsuba(a_plus_b, c_plus_d); });

    for (auto& thread : threads) {
        thread.join();
    }

    // Combine the results
    for (int i = 0; i < n - 1; ++i) {
        ad_bc[i] -= ac[i] + bd[i];
    }

    // Combine the results
    std::vector<int> result(2 * n - 1, 0);

    for (int i = 0; i < n - 1; ++i) {
        result[i] += ac[i];
        result[i + mid] += ad_bc[i];
        result[i + 2 * mid] += bd[i];
    }

    return result;
}

std::vector<int> karatsuba(const std::vector<int>& poly1, const std::vector<int>& poly2) {
    int n = poly1.size();

    // Base case
    if (n <= 4) {
        return multiplyPolynomials(poly1, poly2, 0, n);
    }

    int mid = n / 2;

    // Divide polynomials into two halves
    std::vector<int> a(poly1.begin(), poly1.begin() + mid);
    std::vector<int> b(poly1.begin() + mid, poly1.end());
    std::vector<int> c(poly2.begin(), poly2.begin() + mid);
    std::vector<int> d(poly2.begin() + mid, poly2.end());

    // Recursive steps
    std::vector<int> ac = karatsuba(a, c);
    std::vector<int> bd = karatsuba(b, d);

    std::vector<int> a_plus_b(n - mid);
    std::vector<int> c_plus_d(n - mid);

    for (int i = 0; i < n - mid; ++i) {
        a_plus_b[i] = a[i] + b[i];
        c_plus_d[i] = c[i] + d[i];
    }

    std::vector<int> ad_bc = karatsuba(a_plus_b, c_plus_d);
    for (int i = 0; i < n - 1; ++i) {
        ad_bc[i] -= ac[i] + bd[i];
    }

    // Combine the results
    std::vector<int> result(2 * n - 1, 0);

    for (int i = 0; i < n - 1; ++i) {
        result[i] += ac[i];
        result[i + mid] += ad_bc[i];
        result[i + 2 * mid] += bd[i];
    }

    return result;
}


// Parallelized O(n^2) Polynomial Multiplication
std::vector<int> parallelMultiplyPolynomials(const std::vector<int>& poly1, const std::vector<int>& poly2, int num_threads) {
    int n = poly1.size();
    int m = poly2.size();
    std::vector<int> result(n + m - 1, 0);

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        int start = (t * n) / num_threads;
        int end = ((t + 1) * n) / num_threads;

        threads.emplace_back([&, start, end]() {
            std::vector<int> partialResult = multiplyPolynomials(poly1, poly2, start, end);

            // Synchronize to avoid data race
            for (int i = 0; i < n + m - 1; ++i) {
                result[i] += partialResult[i];
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

const std::string nPrefix = "-n";

int main(int nrArgs, char* args[]) {
	int n = 1024;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		}
	}

	std::vector poly1(n, 1);
	std::vector poly2(n, 1);

	for (int i = 0; i < n; i++) {
		poly1[i] = rand() % 10 + 1;
		poly2[i] = rand() % 10 + 1;
	}

	auto startTime = clock();

	auto result = multiplyPolynomials(poly1, poly2, 0, n);

	auto endTime = clock();

	int ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Sequential: " << ms << " ms\n";
	// for (auto& element : result) {
	// 	std::cout << element << " ";
	// }
	// std::cout << "\n";


	startTime = clock();

	result = parallelMultiplyPolynomials(poly1, poly2, 8);

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Parallel: " << ms << " ms\n";
	// for (auto& element : result) {
	// 	std::cout << element << " ";
	// }
	// std::cout << "\n";


	startTime = clock();

	result = karatsuba(poly1, poly2);

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Karatsuba: " << ms << " ms\n";
	// for (auto& element : result) {
	// 	std::cout << element << " ";
	// }
	// std::cout << "\n";


	startTime = clock();

	result = parallelKaratsuba(poly1, poly2);

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Parallel Karatsuba: " << ms << " ms\n";
	// for (auto& element : result) {
	// 	std::cout << element << " ";
	// }
	// std::cout << "\n";
}