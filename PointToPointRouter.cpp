#include "provided.h"
#include <list>
#include <queue>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <map>
using namespace std;

template <>
struct hash<GeoCoord> {
    size_t operator()(GeoCoord a) const {
        std::hash<std::string> hash;
        return hash(a.latitudeText + a.longitudeText);
    }
};

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;

private:
    const StreetMap* m_sm;

    double h(const GeoCoord& a, const GeoCoord& goal) const {
        //return 0;
        return distanceEarthMiles(a, goal);
    }

    double getWeight(const StreetSegment& a) const {
        return distanceEarthMiles(a.start, a.end);
    }

    void reconstructPath(unordered_map<GeoCoord, StreetSegment> cameFrom, GeoCoord end, list<StreetSegment>& path, double& totalDistance) const {
        totalDistance = 0;
        path.clear();
        StreetSegment current = cameFrom[end];
        path.push_front(current);
        totalDistance += getWeight(current);

        while (cameFrom.find(current.start) != cameFrom.end()) {
            current = cameFrom[current.start];
            path.push_front(current);
            totalDistance += getWeight(current);
        }
    }
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_sm = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    typedef std::pair<double, GeoCoord> PriorityNode;
    std::priority_queue<PriorityNode, std::vector<PriorityNode>, std::greater<PriorityNode>> openSet;
    unordered_set<GeoCoord> closedSet;
    unordered_map<GeoCoord, StreetSegment> cameFrom;
    unordered_map<GeoCoord, double> gScore;

    openSet.emplace(0, start);
    gScore[start] = 0;

    vector<StreetSegment> neighbors;
    if (!m_sm->getSegmentsThatStartWith(start, neighbors)) return BAD_COORD;
    if (!m_sm->getSegmentsThatStartWith(end, neighbors)) return BAD_COORD;

    while (!openSet.empty()) {
        GeoCoord current = openSet.top().second;
        openSet.pop();
        if (current == end) {
            // done
            reconstructPath(cameFrom, current, route, totalDistanceTravelled);
            return DELIVERY_SUCCESS;
        }

        closedSet.insert(current);

        if (!m_sm->getSegmentsThatStartWith(current, neighbors))
            return NO_ROUTE;

        for (auto it = neighbors.begin(); it != neighbors.end(); it++) {
            double cost = gScore[current] + getWeight(*it);
            
            if ((gScore.find(it->end) == gScore.end() && closedSet.find(it->end) == closedSet.end())
                || cost < gScore[it->end]) {
                // not in open or closed
                gScore[it->end] = cost;
                double priority = gScore[it->end] + h(it->end, end);
                openSet.emplace(priority, it->end);
                cameFrom[it->end] = *it;
            }
        }
    }

    return NO_ROUTE;
}



//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
