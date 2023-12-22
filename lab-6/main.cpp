#include <iostream>
#include <vector>
#include <memory>
#include <string>


class Vertex;
class Edge;

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
};

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

class Graph {
	public:
		// std::vector<Vertex> vertices;
		// std::vector<Edge> edges;
		std::vector<std::shared_ptr<Vertex>> vertices;
		std::vector<std::shared_ptr<Edge>> edges;

		Graph(int numVertices, int density) {
			for (int i = 0; i < numVertices; ++i) {
				vertices.emplace_back(std::make_shared<Vertex>(i));
			}

			for (int i = 0; i < numVertices; ++i) {
				for (int j = 0; j < numVertices; ++j) {
					if (i != j && (rand() % 100) < density) {
						auto edge = std::make_shared<Edge>(std::make_shared<Vertex>(vertices[i]), std::make_shared<Vertex>(vertices[j]));
						edges.push_back(edge);
						vertices[i]->outEdges.push_back(edge);
						vertices[j]->inEdges.push_back(edge);
					}
				}
			}
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
};


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
				throw std::runtime_error("Missing value for -m (must be a number)");
			}
			d = std::stoi(args[i + 1]);
		}
	}


	Graph graph(n, d);
	graph.print();
}