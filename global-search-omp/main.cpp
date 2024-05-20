#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include <exception>
#include <set>
#include <map>
#include <chrono>
#include <tuple>
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
    std::vector<Route> routes;
    std::vector<std::tuple<std::set<int>, int, int, int, std::vector<int>>> stack;

public:
    int lowerCost = INT_MAX;
    std::vector<int> route;
    int vehicleCapacity;
    int maxCitiesPerRoute;
    int numberOfRoads;
    int numberOfCities;
    Route bestRoute;
    VRPSolver(const std::string &filename)
    {
        readInput(filename);
        cities.insert(cities.begin(), {0, 0}); // Insert a dummy city representing the depot
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

#ifdef DEBUG
        std::cout << "Number of cities: " << numCities << std::endl;
#endif
        numberOfCities = numCities + 1;

        for (int i = 1; i <= numCities; ++i)
        {
            file >> cities[i].number >> cities[i].package_weight;
        }

        file >> numRoutes;
#ifdef DEBUG
        std::cout << "Number of routes: " << numRoutes << std::endl;
#endif
        numberOfRoads = numRoutes;

        for (int i = 0; i < numRoutes; ++i)
        {
            int start, end, cost;
            file >> start >> end >> cost;
            roads.push_back(Road(cities[start], cities[end], cost));
        }

        file.close();
    }

    void solve()
    {
        std::set<int> citiesVisited;
        citiesVisited.insert(0);
        std::vector<int> route{0};
        stack.emplace_back(citiesVisited, 0, 0, 0, route);

#pragma omp parallel
        {
#pragma omp single nowait
            {
                generateAllPossibleRoutesLoop();
            }
        }

        std::set<Route> filteredRoutes = filterValidRoutes();

#pragma omp parallel for
        for (int i = 0; i < filteredRoutes.size(); ++i)
        {
            auto route = std::next(filteredRoutes.begin(), i);
            int cost = calculateRouteCost(*route);

#pragma omp critical
            {
                if (cost < lowerCost)
                {
                    lowerCost = cost;
                    bestRoute = *route;
                }
            }
        }
    }

    void generateAllPossibleRoutesLoop()
    {
        while (true)
        {
            std::tuple<std::set<int>, int, int, int, std::vector<int>> current;
#pragma omp critical
            {
                if (stack.empty())
                {
                    current = std::tuple<std::set<int>, int, int, int, std::vector<int>>();
                }
                else
                {
                    current = stack.back();
                    stack.pop_back();
                }
            }

            if (std::get<0>(current).empty())
            {
                break;
            }

            auto [placesVisited, numberOfPlacesVisited, previousCity, vehicleLoad, route] = current;

            for (const auto &city : cities)
            {
                int currentCity = city.number;

                if (currentCity == previousCity)
                {
                    continue;
                }

                if (placesVisited.find(currentCity) != placesVisited.end() && currentCity != 0)
                {
                    continue;
                }

                if (currentCity != 0)
                {
                    bool loadExceeded = vehicleLoad + city.package_weight > vehicleCapacity;
                    bool placesExceeded = (numberOfPlacesVisited + 1) > maxCitiesPerRoute;
                    if (loadExceeded || placesExceeded)
                    {
                        continue;
                    }
                }

                auto newPlacesVisited = placesVisited;
                newPlacesVisited.insert(currentCity);
                auto newRoute = route;
                newRoute.push_back(currentCity);

                if (currentCity == 0)
                {
                    if (newPlacesVisited.size() == numberOfCities)
                    {
#pragma omp critical
                        routes.push_back(newRoute);
                    }
#pragma omp critical
                    stack.emplace_back(newPlacesVisited, 0, currentCity, 0, newRoute);
                }
                else
                {
#pragma omp critical
                    stack.emplace_back(newPlacesVisited, numberOfPlacesVisited + 1, currentCity, vehicleLoad + city.package_weight, newRoute);
                }
            }
        }
    }

    std::set<Route> filterValidRoutes()
    {
        std::set<Route> validRoutes;

#pragma omp parallel
        {
            std::set<Route> threadValidRoutes;
#pragma omp for nowait
            for (size_t i = 0; i < routes.size(); ++i)
            {
                const Route &route = routes[i];
                bool routeIsValid = true;
                for (size_t j = 0; j < route.size() - 1; ++j)
                {
                    int source = route[j];
                    int destination = route[j + 1];
                    bool found = std::any_of(roads.begin(), roads.end(), [source, destination](const Road &road)
                                             { return road.start.number == source && road.destination.number == destination; });

                    if (!found)
                    {
                        routeIsValid = false;
                        break;
                    }
                }
                if (routeIsValid)
                {
                    threadValidRoutes.insert(route);
                }
            }
#pragma omp critical
            validRoutes.insert(threadValidRoutes.begin(), threadValidRoutes.end());
        }

        return validRoutes;
    }

    int calculateRouteCost(Route route)
    {
        int cost = 0;
        for (size_t i = 0; i < route.size() - 1; ++i)
        {
            int source = route[i];
            int destination = route[i + 1];

            for (const auto &road : roads)
            {
                if (road.start.number == source && road.destination.number == destination)
                {
                    cost += road.cost;
                    break;
                }
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
        if (argc > 2)
        {
            solver.vehicleCapacity = std::stoi(argv[2]);
            solver.maxCitiesPerRoute = std::stoi(argv[3]);
        }
        else
        {
            solver.getUserInput();
        }

        std::cout << "Starting solver for " << solver.numberOfCities - 1 << " cities and " << solver.numberOfRoads << " routes..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        solver.solve();
        auto endTime = std::chrono::high_resolution_clock::now();

        std::cout << "Lower cost: " << solver.lowerCost << std::endl;

        int lastCityIndex = solver.bestRoute.size() - 1;
        int counter = 0;
        for (const auto &city : solver.bestRoute)
        {
            std::cout << city;
            if (counter < lastCityIndex)
            {
                std::cout << " -> ";
            }
            counter++;
        }
        std::cout << std::endl;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
