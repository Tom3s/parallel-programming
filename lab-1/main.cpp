#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <sstream>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <ctime>
#include <cstdlib>

const std::string nPrefix = "-n";

struct Product {
	int id;
	std::string name;
	int price;
	std::atomic<int> count = 100;

	static int productIdCounter;
	static std::mutex productMutex;

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
	}
};

int Product::productIdCounter = 0;
std::mutex Product::productMutex;

std::vector<Product> PRODUCT_LIST = {
	{"Tomato",      2},
	{"Potato",      4},
	{"Onion",       7},
	{"Carrot",      3},
	{"Cucumber",    5},
	{"Pepper",      8},
	{"Eggplant",    9},
	{"Garlic",      1},
	{"Pumpkin",     6},
	{"Radish",      10},
	{"Cabbage",     5},
	{"Broccoli",    7},
	{"Cauliflower", 8},
	{"Celery",      9},
	{"Spinach",     3},
	{"Lettuce",     4},
	{"Leek",        6},
	{"Asparagus",   2},
	{"Beetroot",    1},
	{"Turnip",      10},
	{"Zucchini",    9},
	{"Mushroom",    8},
	{"Parsley",     7},
	{"Dill",        6},
	{"Basil",       5},
	{"Rosemary",    4},
	{"Thyme",       3},
	{"Sage",        2},
	{"Oregano",     1},
	{"Mint",        10},
	{"Lemon",       9},
	{"Orange",      8},
	{"Apple",       7},
	{"Banana",      6},
	{"Pear",        5},
	{"Peach",       4},
	{"Plum",        3},
	{"Cherry",      2},
	{"Grape",       1},
	{"Watermelon",  10},
	{"Melon",       9},
	{"Strawberry",  8},
	{"Raspberry",   7},
	{"Blueberry",   6},
	{"Blackberry",  5},
	{"Cranberry",   4},
	{"Pineapple",   3},
	{"Kiwi",        2},
	{"Mango",       1},
	{"Papaya",      10},
	{"Coconut",     9},
	{"Avocado",     8},
	{"Grapefruit",  7},
	{"Pomegranate", 6},
	{"Apricot",     5},
	{"Nectarine",   4},
	{"Fig",         3},
	{"Date",        2},
	{"Cherry",      1},
	{"Lime",        10},
	{"Lychee",      9},
	{"Passion",     8},
	{"Dragon",      7},
	{"Guava",       6},
	{"Jackfruit",   5},
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

void sellProduct(std::vector<int> productIds) {
	while (isVerificationInProgress) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::shared_lock<std::shared_mutex> readLock(verificationMutex);

	std::vector<int> billedProducts;

	salesRunning++;
	for (auto& productId : productIds){	
		auto& product = PRODUCT_LIST[productId];

		std::lock_guard<std::mutex> lock(product.productMutex);

		if (product.count <= 0) {
			continue;
		}

		product.decreaseCount(1);

		billedProducts.push_back(productId);

		totalProfit += product.price;
	}
	std::lock_guard<std::mutex> lock(billsMutex);
	bills.push_back(createBill(billedProducts));
	salesRunning--;
}

std::vector<int> initialProductCounts;

void verifyBills() {
	std::unique_lock<std::shared_mutex> writeLock(verificationMutex);
	isVerificationInProgress = true;
	while (salesRunning > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	int profitByBill = 0;

	std::vector<int> productCountByBill(initialProductCounts);

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
	std::srand(std::time(nullptr));

	auto n = 100;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		}
	}

	for (auto& product : PRODUCT_LIST) {
		auto newCount = std::rand() % n + 100 * n;
		product.count = newCount;
		initialProductCounts.push_back(product.count);
	}

	std::vector<std::thread> threads;

	for (int i = 0; i < n; i++) {
		// threads.emplace_back(printHello, i);

		std::vector<int> productsToSell;

		for (int j = 0; j < PRODUCT_LIST.size(); j++) {
			if (PRODUCT_LIST[j].count <= 0) {
				continue;
			}
			int count = std::rand() % std::max(1, PRODUCT_LIST[j].count / n) + 1;
			auto productIds = std::vector<int>(count, j);
			productsToSell.insert(productsToSell.end(), productIds.begin(), productIds.end());
		}

		threads.emplace_back(sellProduct, productsToSell);

		if (i % (n / 10) == 0) {
			threads.emplace_back(verifyBills);
		}
	}

	for (auto& t : threads) {
		t.join();
	}

	std::cout << "Total profit: " << totalProfit << std::endl;
	for (int i = 0; i < PRODUCT_LIST.size(); i++) {
		std::cout << PRODUCT_LIST[i].name << ": "<< initialProductCounts[i] << " -> " << PRODUCT_LIST[i].count << std::endl;
	}
	std::cout << std::flush;

	return 0;
}