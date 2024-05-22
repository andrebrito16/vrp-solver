#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include <exception>
#include <set>
#include <map>
#include <chrono>
#include <omp.h>
#include <climits>

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

    VRPSolver(const std::string &filename, int vehicleCapacity, int maxCitiesPerRoute)
        : vehicleCapacity(vehicleCapacity), maxCitiesPerRoute(maxCitiesPerRoute)
    {
        readInput(filename);
        cities.insert(cities.begin(), {0, 0});
        buildAdjacencyList();
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

#pragma omp parallel
        {
            std::vector<Route> localBestRoutes;
            int localTotalCost = 0;

#pragma omp for schedule(dynamic)
            for (int i = 0; i < cities.size() - 1; ++i)
            {
                std::set<int> localVisitedCities = visitedCities;
                Route route = findNextRoute(localVisitedCities, 0);
                if (route.empty())
                {
                    continue;
                }
                route.insert(route.begin(), 0);
                route.push_back(0);
                route = twoOpt(route);
                int routeCost = calculateRouteCost(route);
                localTotalCost += routeCost;
                localBestRoutes.push_back(route);
            }

#pragma omp critical
            {
                totalCost += localTotalCost;
                bestRoutes.insert(bestRoutes.end(), localBestRoutes.begin(), localBestRoutes.end());
            }
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

    Route twoOpt(const Route &route)
    {
        Route newRoute = route;
        int bestCost = calculateRouteCost(route);
        bool improvement = true;

        while (improvement)
        {
            improvement = false;

#pragma omp parallel for schedule(dynamic)
            for (size_t i = 1; i < route.size() - 2; ++i)
            {
                for (size_t j = i + 1; j < route.size() - 1; ++j)
                {
                    Route candidate = twoOptSwap(newRoute, i, j);
                    int candidateCost = calculateRouteCost(candidate);
                    if (candidateCost < bestCost)
                    {
#pragma omp critical
                        {
                            newRoute = candidate;
                            bestCost = candidateCost;
                            improvement = true;
                        }
                    }
                }
            }
        }

        return newRoute;
    }

    Route twoOptSwap(const Route &route, int i, int k)
    {
        Route newRoute(route.begin(), route.begin() + i);
        std::reverse_copy(route.begin() + i, route.begin() + k + 1, std::back_inserter(newRoute));
        std::copy(route.begin() + k + 1, route.end(), std::back_inserter(newRoute));
        return newRoute;
    }
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 4)
        {
            std::cerr << "Usage: " << argv[0] << " <input file> <vehicle capacity> <max cities per route>" << std::endl;
            return 1;
        }

        std::string filename = argv[1];
        int vehicleCapacity = std::stoi(argv[2]);
        int maxCitiesPerRoute = std::stoi(argv[3]);

        VRPSolver solver(filename, vehicleCapacity, maxCitiesPerRoute);

        auto startTime = std::chrono::high_resolution_clock::now();
        solver.solve();
        auto endTime = std::chrono::high_resolution_clock::now();

        std::cout << "Lower cost: " << solver.lowerCost << std::endl;

        int routesLength = solver.bestRoutes.size();
        for (const Route &route : solver.bestRoutes)
        {
            for (int city : route)
            {
                std::cout << city << " ";
            }
            if (--routesLength > 0)
            {
                std::cout << "-> ";
            }
        }
        std::cout << std::endl;
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

        std::cout << "Time taken: " << duration << " nanosseconds" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
