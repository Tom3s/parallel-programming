#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

const double G = 6.67430e-11;  // Gravitational constant

class Vector2 {
public:
	double x, y;

	Vector2(double x, double y) : x(x), y(y) {}

	Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator*(double scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	Vector2 operator/(double scalar) const {
		return Vector2(x / scalar, y / scalar);
	}

	double length() const {
		return std::sqrt(x * x + y * y);
	}
};

class Body;

std::vector<Body> bodies;

class Body {
public:
    double mass;
    // double x, y;  // Position
    // double vx, vy;  // Velocity
	Vector2 position;
	Vector2 velocity;

    // Body(double mass, double x, double y, double vx, double vy)
    //     : mass(mass), x(x), y(y), vx(vx), vy(vy) {}
	Body (double mass, Vector2 position, Vector2 velocity) 
		: mass(mass), position(position), velocity(velocity) {}

    // Update the position and velocity of the body for a small time step (dt)
    void calculateVelocity(double delta) {
        // Calculate total acceleration due to gravitational forces from other bodies
		Vector2 acceleration = Vector2(0.0, 0.0);
        for (const Body& other : bodies) {
            if (&other != this) {  // Exclude self-interaction
				Vector2 distance = other.position - position;
				double r = distance.length();
                double f = (G * mass * other.mass) / (r * r);  // Gravitational force magnitude
				acceleration = acceleration + (distance / r) * f / mass;
            }
        }

        // Update velocity using the calculated acceleration
		velocity = velocity + acceleration * delta;

    }

	void updatePosition(double delta) {
        // Update position using current velocity
		position = position + velocity * delta;
	}
};

#include <barrier>

// std::barrier threadBarrier;

void updateTask(Body& body, double delta, int nrSteps, std::barrier<>& threadBarrier) {
	for (int i = 0; i < nrSteps; i++) {
		body.calculateVelocity(delta);

		threadBarrier.arrive_and_wait();

		body.updatePosition(delta);

		threadBarrier.arrive_and_wait();
	}

}

// Vector to store all the bodies in the simulation

// Generate a random number in the range [min, max)
double randomRange(double min, double max) {
	return min + (max - min) * (double)rand() / RAND_MAX;
}

#include "BS_thread_pool_light.hpp"

#include <chrono>

using namespace std::chrono;
#define clock high_resolution_clock::now

const std::string nPrefix = "-n";
const std::string tPrefix = "-t";


int main(int nrArgs, char *args[])
{
	std::srand(time(NULL));

	auto n = 5;
	auto NR_TICKS = 5000;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		} else if (args[i] == tPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -t (must be a number)");
			}
			NR_TICKS = std::stoi(args[i + 1]);
		}
	}

	// Initialize bodies with mass, initial position, and initial velocity
	// bodies.push_back(Body(1.0e12, Vector2(0.0, 0.0), Vector2(0.0, 0.0)));
	// bodies.push_back(Body(1.0e12, Vector2(1.0, 0.0), Vector2(0.0, 1.0)));
	for (int i = 0; i < n; i++) {
		auto mass = randomRange(1.0e11, 1.0e13);
		auto x = randomRange(-100.0, 100.0);
		auto y = randomRange(-100.0, 100.0);
		auto vx = randomRange(-10.0, 10.0);
		auto vy = randomRange(-10.0, 10.0);
		bodies.emplace_back(mass, Vector2(x, y), Vector2(vx, vy));
	}

	auto initialBodies = std::vector<Body>(bodies);

    // // Simulation parameters
	const double delta = 0.01;  // Time step


	auto startTime = clock();
    // Run the simulation for a certain number of steps
    for (int step = 0; step < NR_TICKS; ++step) {
        // Update each body in the simulation
        for (Body& body : bodies) {
            // body.update(delta);
			body.calculateVelocity(delta);

        }
		for (Body& body : bodies) {
			body.updatePosition(delta);
		}
    }
	auto endTime = clock();

	std::cout << "Sequential algorithm:\n";
	std::cout << n << " bodies for " << NR_TICKS << " ticks took " << duration_cast<milliseconds>(endTime - startTime).count() << "ms\n";

	// threaded version

	bodies = initialBodies;

	std::barrier threadBarrier(n);

	startTime = clock();

	// Run the simulation for a certain number of steps
	BS::thread_pool_light pool(n);

	for (int i = 0; i < n; i++) {
		pool.push_task(updateTask, std::ref(bodies[i]), delta, NR_TICKS, std::ref(threadBarrier));
	}

	pool.wait_for_tasks();

	endTime = clock();

	std::cout << "Threaded algorithm:\n";
	std::cout << n << " bodies for " << NR_TICKS << " ticks took " << duration_cast<milliseconds>(endTime - startTime).count() << "ms\n";

    return 0;
}