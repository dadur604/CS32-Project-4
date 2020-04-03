#include "provided.h"
#include <vector>
#include <random>
using namespace std;


std::default_random_engine random_engine;

double randDouble(double min, double max) {
    std::uniform_real_distribution<double> unif(min, max);

    double rand = unif(random_engine);
    return rand;
}

int randInt(int min, int max) {
    std::uniform_int_distribution<int> uni(min, max);

    int rand = uni(random_engine);
    return rand;
}

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    vector<DeliveryRequest> getNeighbor(const vector<DeliveryRequest>& oldState) const;

    double E(const GeoCoord& depot, const vector<DeliveryRequest>& state) const;
    double P(double E1, double E2, double Temp) const;

    const int kMax = 100;
    const double TMin = 0.0001;

};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = 0;
    oldCrowDistance += distanceEarthMiles(depot, deliveries[0].location);
    for (auto it = deliveries.begin(); it != deliveries.end()-1; it++) {
        oldCrowDistance += distanceEarthMiles(it->location, (it+1)->location);
    }
    oldCrowDistance += distanceEarthMiles(deliveries.rbegin()->location, depot);

    // Simulated Annealing!!
    vector<DeliveryRequest> state = deliveries;
    vector<DeliveryRequest> newState;
    double T = 1;
    while (T >= TMin) {
        for (double k = 0; k < kMax; k++) {
            newState = getNeighbor(state);
            if (P(E(depot, state), E(depot, newState), T) >= randDouble(0, 1))
                state = newState;
        }
        T *= .9;
    }

    deliveries = state;

    // New Crow Distance
    newCrowDistance = 0;
    newCrowDistance += distanceEarthMiles(depot, deliveries[0].location);
    for (auto it = deliveries.begin(); it != deliveries.end() - 1; it++) {
        newCrowDistance += distanceEarthMiles(it->location, (it + 1)->location);
    }
    newCrowDistance += distanceEarthMiles(deliveries.rbegin()->location, depot);

    return;
}

double DeliveryOptimizerImpl::E(const GeoCoord& depot, const vector<DeliveryRequest>& state) const {
    double distance = 0;
    distance += distanceEarthMiles(depot, state[0].location);
    for (auto it = state.begin(); it != state.end() - 1; it++) {
        distance += distanceEarthMiles(it->location, (it + 1)->location);
    }
    distance += distanceEarthMiles(state.rbegin()->location, depot);
    return distance;
}

double DeliveryOptimizerImpl::P(double E1, double E2, double Temp) const {
    double P;
    P = exp((E1 - E2) / Temp);
    return P;
}

vector<DeliveryRequest> DeliveryOptimizerImpl::getNeighbor(const vector<DeliveryRequest>& oldState) const {
    vector<DeliveryRequest> newState = oldState;

    int indexA = randInt(0, oldState.size() - 1); // get the indexes of nodes to swap
    int indexB = randInt(0, oldState.size() - 1);

    auto itA = newState.begin() + indexA;
    auto itB = newState.begin() + indexB;

    std::iter_swap(itA, itB);

    return newState;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
