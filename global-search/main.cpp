#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include <exception>
#include <set>
#include <map>

// #define DEBUG2

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
    int numberOfCities;
    std::vector<Route> routes;

public:
    int lowerCost = INT_MAX;
    std::vector<int> route;
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

        std::cout << "Number of cities: " << numCities << std::endl;
        numberOfCities = numCities + 1;

#ifdef DEBUG
        std::cout << "Warehouse " << cities[0].number << " with weight " << cities[0].package_weight << std::endl;
#endif

        for (int i = 1; i <= numCities; ++i)
        {
            file >> cities[i].number >> cities[i].package_weight;
#ifdef DEBUG
            std::cout << "City " << cities[i].number << " with weight " << cities[i].package_weight << std::endl;
#endif
        }

        file >> numRoutes;
        std::cout << "Number of routes: " << numRoutes << std::endl;

        for (int i = 0; i < numRoutes; ++i)
        {
            int start, end, cost;
            file >> start >> end >> cost;

            roads.push_back(Road(cities[start], cities[end], cost));
        }

#ifdef DEBUG
        for (const auto &road : roads)
        {
            std::cout << "Road from " << road.start.number << " to " << road.destination.number << " with cost " << road.cost << std::endl;
        }
#endif

        file.close();
    }

    void solve()
    {
        std::set<int> citiesVisited;
        citiesVisited.insert(0);
        std::vector<int> route{0};
        generateAllPossibleRoutes(citiesVisited, 0, 0, 0, route);

        std::set<Route> filteredRoutes = filterValidRoutes();

        for (const auto &route : filteredRoutes)
        {
            int cost = calculateRouteCost(route);

#ifdef DEBUG2
            if (route[1] == 1)
            {
                std::cout << "Route: ";
                for (const auto &city : route)
                {
                    std::cout << city << " ";
                }
                std::cout << "Cost: " << cost << std::endl;
            }
#endif
            if (cost < lowerCost)
            {
                lowerCost = cost;
                bestRoute = route;
            }
        }
    }

    void generateAllPossibleRoutes(std::set<int> &placesVisited, int numberOfPlacesVisited, int previousCity, int vehicleLoad, std::vector<int> &route)
    {
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

            placesVisited.insert(currentCity);
            route.push_back(currentCity);

            if (currentCity == 0)
            {
                if (placesVisited.size() == numberOfCities)
                {
                    routes.push_back(route);
                }
                generateAllPossibleRoutes(placesVisited, 0, currentCity, 0, route);
            }
            else
            {
                generateAllPossibleRoutes(placesVisited, numberOfPlacesVisited + 1, currentCity, vehicleLoad + city.package_weight, route);
            }
            route.pop_back();
            placesVisited.erase(currentCity);
        }
    }

    std::set<Route> filterValidRoutes()
    {
        std::set<Route> validRoutes;

        for (const Route &route : routes)
        {
            bool routeIsValid = true;
            for (size_t i = 0; i < route.size() - 1; ++i)
            {
                int source = route[i];
                int destination = route[i + 1];
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
                validRoutes.insert(route);
            }
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
        solver.getUserInput();

        std::cout << "Solver created successfully." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        solver.solve();
        auto endTime = std::chrono::high_resolution_clock::now();

        std::cout << "Lower cost: " << solver.lowerCost << std::endl;

        for (const auto &city : solver.bestRoute)
        {
            std::cout << city << " ";
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
