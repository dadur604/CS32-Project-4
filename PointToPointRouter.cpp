#include "provided.h"
#include <list>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>
#include <functional>
#include <algorithm>
using namespace std;

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

    vector<GeoCoord> reconstructPath(map<GeoCoord, GeoCoord> cameFrom, GeoCoord current) const {
        vector<GeoCoord> path;
        path.push_back(current);
        while (cameFrom.find(current) != cameFrom.end()) {
            current = cameFrom[current];
            path.push_back(current);
        }
        return path;
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
    //A* Algorithm!
    map<GeoCoord, GeoCoord> cameFrom;

    map<GeoCoord, double> gScore;
    gScore[start] = 0;

    map<GeoCoord, double> fScore;
    fScore[start] = h(start, end); // implement h

    auto comp = [&fScore = fScore](GeoCoord a, GeoCoord b) {return fScore[a] < fScore[b]; };
    //priority_queue < GeoCoord, std::vector<GeoCoord>, decltype(comp)> openSet(comp);
    set<GeoCoord, decltype(comp)> openSet(comp);
    openSet.insert(start);

    GeoCoord current;
    while (!openSet.empty()) {
        //current = openSet.top();
        current = *openSet.begin();
        cout << current.latitudeText << " " << current.longitudeText << endl;
        if (current == end) {
            // reconstruct path (cameFrom, current)
            // success
            cerr << "hi" << endl;
            auto a = reconstructPath(cameFrom, current);
        }

        //openSet.pop();
        openSet.erase(openSet.begin());
        // For each neighborh of current
        vector<StreetSegment> segs;
        m_sm->getSegmentsThatStartWith(current, segs);

        for (auto it = segs.begin(); it != segs.end(); it++) {
            // d(current,neighbor) is the weight of the edge from current to neighbor
            // tentative_gScore is the distance from start to the neighbor through current
            double tentative_gScore = gScore[current] + distanceEarthMiles(current, it->end);
            if (auto g = gScore.find(it->end) == gScore.end()) {
                gScore[it->end] = DBL_MAX;
            }
            if (tentative_gScore < gScore[it->end]) { // fix this
                cameFrom[it->end] = current;
                gScore[it->end] = tentative_gScore;
                fScore[it->end] = gScore[it->end] + h(it->end, end);
                //openSet.push(it->end); // only if unique!
                openSet.insert(it->end);
            }

        }
    }

    // failure
    return DELIVERY_SUCCESS;
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
