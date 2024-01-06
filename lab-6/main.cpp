#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <mutex>
#include <thread>



class Vertex;

class Edge {
	public:
		std::shared_ptr<Vertex> from;
		std::shared_ptr<Vertex> to;
		// int weight;
		Edge(std::shared_ptr<Vertex> from, std::shared_ptr<Vertex> to) : from(from), to(to) {}

		Edge(Edge* edge) {
			from = edge->from;
			to = edge->to;
		}

		Edge(std::shared_ptr<Edge> edge) {
			from = edge->from;
			to = edge->to;
		}
};

class Vertex {
	public:
		int id;
		std::vector<std::shared_ptr<Edge>> inEdges;
		std::vector<std::shared_ptr<Edge>> outEdges;
		Vertex(int id) : id(id) {}

		Vertex(Vertex* vertex) {
			id = vertex->id;
			inEdges = vertex->inEdges;
			outEdges = vertex->outEdges;
		}

		Vertex(std::shared_ptr<Vertex> vertex) {
			id = vertex->id;
			inEdges = vertex->inEdges;
			outEdges = vertex->outEdges;
		}

		bool hasOutEdgeTo(int vertexId) {
			for (auto& edge : outEdges) {
				if (edge->to->id == vertexId) {
					return true;
				}
			}
			return false;
		}

		bool hasInEdgeFrom(int vertexId) {
			for (auto& edge : inEdges) {
				if (edge->from->id == vertexId) {
					return true;
				}
			}
			return false;
		}
};



class Graph {
	public:
		// std::vector<Vertex> vertices;
		// std::vector<Edge> edges;
		std::vector<std::shared_ptr<Vertex>> vertices;
		std::vector<std::shared_ptr<Edge>> edges;

		int startingVertex = -1;

		Graph(int numVertices, int density) {
			for (int i = 0; i < numVertices; ++i) {
				vertices.emplace_back(std::make_shared<Vertex>(i));
			}

			for (int i = 0; i < numVertices; ++i) {
				for (int j = 0; j < numVertices; ++j) {
					if (i != j && (rand() % 1000) < density) {
						auto edge = std::make_shared<Edge>(std::make_shared<Vertex>(vertices[i]), std::make_shared<Vertex>(vertices[j]));
						edges.push_back(edge);
						vertices[i]->outEdges.push_back(edge);
						vertices[j]->inEdges.push_back(edge);
					}
				}
			}

			startingVertex = getVertexWithMostOutEdges();
		}

		void print() {
			for (auto& vertex : vertices) {
				std::cout << vertex->id << ": ";
				for (auto& edge : vertex->outEdges) {
					std::cout << edge->to->id << " ";
				}
				std::cout << std::endl;
			}
		}

		int getVertexWithMostOutEdges() {
			if (startingVertex != -1) return startingVertex;

			int max = 0;
			int vertexId = 0;
			for (auto& vertex : vertices) {
				if (vertex->outEdges.size() > max) {
					max = vertex->outEdges.size();
					vertexId = vertex->id;
				}
			}
			return vertexId;
		}
};

bool isValid(std::vector<int>& path, Vertex& vertex) {
	for (auto& id : path) {
		if (id == vertex.id) {
			return false;
		}
	}

	if (path.size() == 0 || vertex.hasInEdgeFrom(path.back())) {
		return true;
	}

	return false;
}

bool hamiltonUtil(Graph& graph, std::vector<int>& path, int pos) {
	if (pos == graph.vertices.size()) {
		return graph.vertices[path[pos - 1]]->hasOutEdgeTo(path[0]);
	}

	for (int i = 0; i < graph.vertices.size(); ++i) {
		if (isValid(path, *graph.vertices[i])) {
			path.push_back(graph.vertices[i]->id);
			if (hamiltonUtil(graph, path, pos + 1)) {
				return true;
			}
			path.pop_back();
		}
	}

	return false;
}

std::vector<int> hamilton(Graph& graph) {
	std::vector<int> path;
	path.push_back(graph.startingVertex);
	if (hamiltonUtil(graph, path, 1)) {
		return path;
	}
	return std::vector<int>();
}

std::vector<int> bestPath;
std::mutex pathMutex;

std::vector<int> hamiltonUtilParallel(Graph& graph, std::vector<int> path, int pos) {
	if (pos == graph.vertices.size()) {
		if (graph.vertices[path[pos - 1]]->hasOutEdgeTo(path[0])) {
			std::lock_guard<std::mutex> lock(pathMutex);
			bestPath = path;
			return path;
		}
		return std::vector<int>();
	}

	auto threads = std::vector<std::thread>();

	for (int i = 0; i < graph.vertices.size(); ++i) {
		if (isValid(path, *graph.vertices[i])) {
			path.push_back(graph.vertices[i]->id);
			threads.push_back(std::thread(hamiltonUtilParallel, std::ref(graph), path, pos + 1));
			path.pop_back();
		}		
	}

	for (auto& thread : threads) {
		thread.join();
		return bestPath;
	}

	return std::vector<int>();
}
// void worker(Graph& graph, int startVertex) {
//     std::vector<int> path;
//     path.push_back(startVertex);
//     if (hamiltonUtil(graph, path, 1)) {
//         std::lock_guard<std::mutex> lock(pathMutex);
//         if (bestPath.empty() || path.size() < bestPath.size()) {
//             bestPath = path;
//         }
//     }
// }

// std::vector<int> hamiltonParallel(Graph& graph) {
//     std::vector<std::thread> workers;
//     for (int i = 0; i < graph.vertices.size(); i++) {
//         workers.push_back(std::thread(worker, std::ref(graph), i));
//     }
//     for (auto& worker : workers) {
//         worker.join();
//     }
//     return bestPath;
// }
std::vector<int> hamiltonParallel(Graph& graph) {
	std::vector<int> path;
	path.push_back(graph.startingVertex);
	if (hamiltonUtil(graph, path, 1)) {
		return path;
	}
	return std::vector<int>();
}


#include <chrono>

using namespace std::chrono;
#define clock high_resolution_clock::now


const std::string nPrefix = "-n";
const std::string dPrefix = "-d";

int main(int nrArgs, char* args[]) {

	auto n = 10;
	auto d = 50;

	for (int i = 0; i < nrArgs; i++) {
		if (args[i] == nPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -n (must be a number)");
			}
			n = std::stoi(args[i + 1]);
		} else if (args[i] == dPrefix) {
			if (i + 1 >= nrArgs) {
				throw std::runtime_error("Missing value for -d (must be a number)");
			}
			d = std::stoi(args[i + 1]);
		}
	}


	Graph graph(n, d);

	// graph.print();

	auto startTime = clock();

	auto path = hamilton(graph);

	if (path.size() == 0) {
		std::cout << "No Hamiltonian cycle found\n";
	} else {
		std::cout << "Hamiltonian cycle found: ";
		for (auto& id : path) {
			std::cout << id << " ";
		}
		std::cout << "\n";
	}

	auto endTime = clock();

	int ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Time for linear algorithm: " << ms << " ms\n";


	startTime = clock();

	path = hamiltonParallel(graph);

	if (path.size() == 0) {
		std::cout << "No Hamiltonian cycle found\n";
	} else {
		std::cout << "Hamiltonian cycle found: ";
		for (auto& id : path) {
			std::cout << id << " ";
		}
		std::cout << "\n";
	}

	endTime = clock();

	ms = duration_cast<milliseconds>(endTime - startTime).count();

	std::cout << "Time for parallel algorithm: " << ms << " ms\n";
}