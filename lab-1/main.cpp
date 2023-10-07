#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <sstream>
#include <atomic>
#include <mutex>
#include <shared_mutex>

const std::string nPrefix = "-n";

struct Product {
	int id;
	std::string name;
	int price;
	std::atomic<int> count = 100;

	static int productIdCounter;

	Product() = default;

	Product(std::string name, int price, int count = 100) : name(name), price(price) {
		this->count = count;
		this->id = productIdCounter;		
		productIdCounter++;
	}

	Product(const Product& other)
        : name(other.name), price(other.price), count(other.count.load()) {}

	void decreaseCount(int count) {
		this->count -= count;
		if (this->count <= 0) {
			std::string out = "Ran out of " + this->name + "\n";
			std::cout << out << std::flush;
		}
	}
};

int Product::productIdCounter = 0;

std::vector<Product> PRODUCT_LIST = {
	{"Tomato",      2,  100},
	{"Potato",      4,  100},
	{"Onion",       7,  100},
	{"Carrot",      3,  100},
	{"Cucumber",    5,  100},
	{"Pepper",      8,  100},
	{"Eggplant",    9,  100},
	{"Garlic",      1,  100},
	{"Pumpkin",     6,  100},
	{"Radish",      10, 100},
};
std::mutex productListMutex;

struct Bill {
	std::vector<int> productIds;
	int totalPrice;
};

Bill createBill(std::vector<int> productIds) {
	Bill bill;
	bill.productIds = productIds;
	bill.totalPrice = 0;

	for (auto& product : productIds) {
		bill.totalPrice += PRODUCT_LIST[product].price;
	}

	return bill;
}

std::atomic<int> totalProfit = 0;
std::mutex totalProfitMutex;

std::vector<Bill> bills;
std::mutex billsMutex;

std::atomic<bool> isVerificationInProgress(false);
std::shared_mutex verificationMutex;
std::atomic<int> salesRunning = 0;

void sellProduct(int productId, int count = 1) {
	while (isVerificationInProgress) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::shared_lock<std::shared_mutex> readLock(verificationMutex);

	salesRunning++;
	auto& product = PRODUCT_LIST[productId];

	product.decreaseCount(count);

	// std::lock_guard<std::mutex> profitLock(totalProfitMutex);
	std::lock_guard<std::mutex> lock(billsMutex);

	totalProfit += product.price * count;
	bills.push_back(createBill({productId}));
	salesRunning--;
}

void verifyBills() {
	std::unique_lock<std::shared_mutex> writeLock(verificationMutex);
	isVerificationInProgress = true;
	while (salesRunning > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	int profitByBill = 0;

	std::vector<int> productCountByBill(PRODUCT_LIST.size(), 100);

	for (auto& bill : bills) {
		profitByBill += bill.totalPrice;
		for (auto& productId : bill.productIds) {
			productCountByBill[productId]--;
		}
	}

	std::stringstream ss;

	ss << "Total profit: " << totalProfit << std::endl;
	ss << "Calculated profit: " << profitByBill << std::endl;
	if (totalProfit != profitByBill) {
		ss << "====================== Profit mismatch!!!! ======================" << std::endl;
	}

	for (int i = 0; i < PRODUCT_LIST.size(); i++) {
		if (productCountByBill[i] != PRODUCT_LIST[i].count) {
			ss << PRODUCT_LIST[i].name << ": "<< PRODUCT_LIST[i].count << " != " << productCountByBill[i] << std::endl;
			ss << "====================== Product count mismatch!!!! ======================" << std::endl;
		}
	}

	std::cout << ss.str() << std::flush;
	isVerificationInProgress = false;
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

	std::vector<std::thread> threads;

	for (int i = 0; i < n; i++) {
		// threads.emplace_back(printHello, i);
		for (int j = 0; j < PRODUCT_LIST.size(); j++) {
			threads.emplace_back(sellProduct, j, 1);
		}
		threads.emplace_back(verifyBills);
	}

	for (auto& t : threads) {
		t.join();
	}

	std::cout << "Total profit: " << totalProfit << std::endl;

	return 0;
}