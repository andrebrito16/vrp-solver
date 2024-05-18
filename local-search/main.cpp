#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include <exception>
#include <set>
#include <map>

using Route = std::vector<int>;

struct City
{
    int number;
    int package_weight;

    bool operator<(const City &other) const
    {
        return number < other.number;
    }
};

struct Road
{
    City start;
    City destination;
    int cost;

    Road(City start, City destination, int cost) : start(start), destination(destination), cost(cost) {}
};

class VRPSolver
{
private:
    std::vector<City> cities;
    std::vector<Road> roads;
    int vehicleCapacity;
    int maxCitiesPerRoute;
    std::map<int, std::vector<std::pair<int, int>>> adjList;

public:
    int lowerCost = INT_MAX;
    std::vector<Route> bestRoutes;

    VRPSolver(const std::string &filename)
    {
        readInput(filename);
        cities.insert(cities.begin(), {0, 0});
        buildAdjacencyList();
    }

    void getUserInput()
    {
        std::cout << "Enter vehicle capacity: ";
        std::cin >> vehicleCapacity;
        std::cout << "Enter maximum number of cities vehicle can visit per trip: ";
        std::cin >> maxCitiesPerRoute;
    }

    void readInput(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open input file.");
        }

        int numCities, numRoutes;
        file >> numCities;
        cities.resize(numCities + 1);

        for (int i = 1; i <= numCities; ++i)
        {
            file >> cities[i].number >> cities[i].package_weight;
        }

        file >> numRoutes;

        for (int i = 0; i < numRoutes; ++i)
        {
            int start, end, cost;
            file >> start >> end >> cost;
            roads.push_back(Road(cities[start], cities[end], cost));
        }

        file.close();
    }

    void buildAdjacencyList()
    {
        for (const auto &road : roads)
        {
            adjList[road.start.number].emplace_back(road.destination.number, road.cost);
        }
    }

    void solve()
    {
        std::set<int> visitedCities;
        visitedCities.insert(0);
        int totalCost = 0;

        while (visitedCities.size() < cities.size())
        {
            Route route = findNextRoute(visitedCities, 0);
            if (route.empty())
            {
                break;
            }
            route.insert(route.begin(), 0);
            route.push_back(0);
            int routeCost = calculateRouteCost(route);
            totalCost += routeCost;
            bestRoutes.push_back(route);
        }

        lowerCost = totalCost;
    }

    Route findNextRoute(std::set<int> &visitedCities, int startCity)
    {
        Route route;
        int currentCity = startCity;
        int totalWeight = 0;
        int numCitiesVisited = 0;

        while (numCitiesVisited < maxCitiesPerRoute && visitedCities.size() < cities.size())
        {
            int nextCity = -1;
            int minCost = INT_MAX;

            for (const auto &[neighbor, cost] : adjList[currentCity])
            {
                if (visitedCities.find(neighbor) == visitedCities.end() &&
                    totalWeight + cities[neighbor].package_weight <= vehicleCapacity &&
                    cost < minCost)
                {
                    nextCity = neighbor;
                    minCost = cost;
                }
            }

            if (nextCity == -1)
                break; // No valid next city found

            route.push_back(nextCity);
            visitedCities.insert(nextCity);
            totalWeight += cities[nextCity].package_weight;
            currentCity = nextCity;
            numCitiesVisited++;
        }

        return route;
    }

    int calculateRouteCost(const Route &route)
    {
        int cost = 0;
        for (size_t i = 0; i < route.size() - 1; ++i)
        {
            int source = route[i];
            int destination = route[i + 1];
            auto it = std::find_if(adjList[source].begin(), adjList[source].end(),
                                   [destination](const std::pair<int, int> &p)
                                   { return p.first == destination; });
            if (it != adjList[source].end())
            {
                cost += it->second;
            }
        }
        return cost;
    }
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
            return 1;
        }

        VRPSolver solver(argv[1]);
        solver.getUserInput();

        solver.solve();

        std::cout << "Lower cost: " << solver.lowerCost << std::endl;

        for (const Route &route : solver.bestRoutes)
        {
            for (int city : route)
            {
                std::cout << city << " ";
            }
            std::cout << "-> ";
        }
        std::cout << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
