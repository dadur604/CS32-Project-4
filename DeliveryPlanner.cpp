#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;

    void getCommands(list<StreetSegment> route, list<DeliveryCommand>& commands) const;

    string getDirection(double angle) const {
        if (0 <= angle && angle < 22.5)
            return "east";
        if (22.5 <= angle && angle < 67.5)
            return "northeast";
        if (67.5 <= angle && angle < 112.5)
            return "north";
        if (112.5 <= angle && angle < 157.5)
            return "northwest";
        if (157.7 <= angle && angle < 202.5)
            return "west";
        if (202.5 <= angle && angle < 247.5)
            return "southwest";
        if (247.5 <= angle && angle < 292.5)
            return "south";
        if (292.5 <= angle && angle < 337.5)
            return "southeast";
        if (337.5 <= angle && angle < 360)
            return "east";
        return "INVALID ANGLE";
    }
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_sm = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    totalDistanceTravelled = 0;

    PointToPointRouter router(m_sm);

    commands.clear();

    // optimize deliveries
    DeliveryOptimizer optimizer(m_sm);
    double oldCrow, newCrow;
    vector<DeliveryRequest> optimizedDeliveries = deliveries;
    optimizer.optimizeDeliveryOrder(depot, optimizedDeliveries, oldCrow, newCrow);

    list<StreetSegment> route;
    double totalDistance;

    DeliveryCommand deliverCommand;

    // get route from depot to first delivery
    list<DeliveryCommand> output;

    DeliveryResult result = 
        router.generatePointToPointRoute(depot, optimizedDeliveries[0].location, route, totalDistance);
    if (result != DELIVERY_SUCCESS) return result;

    getCommands(route, output);
    commands.insert(commands.end(), output.begin(), output.end());
    deliverCommand.initAsDeliverCommand(optimizedDeliveries[0].item);
    commands.push_back(deliverCommand);
    totalDistanceTravelled += totalDistance;

    for (auto from = optimizedDeliveries.begin(); from != optimizedDeliveries.end() - 1; from++) {
        // get routes between optimizedDeliveries

        auto to = from + 1;

        result =
            router.generatePointToPointRoute(from->location, to->location, route, totalDistance);
        if (result != DELIVERY_SUCCESS) return result;

        getCommands(route, output);
        commands.insert(commands.end(), output.begin(), output.end());
        deliverCommand.initAsDeliverCommand(to->item);
        commands.push_back(deliverCommand);
        totalDistanceTravelled += totalDistance;
    }

    // get route from last delivery to depot

    result = 
        router.generatePointToPointRoute(optimizedDeliveries.rbegin()->location, depot, route, totalDistance);
    if (result != DELIVERY_SUCCESS) return result;

    getCommands(route, output);
    commands.insert(commands.end(), output.begin(), output.end());
    totalDistanceTravelled += totalDistance;

    return DELIVERY_SUCCESS;
}

void DeliveryPlannerImpl::getCommands(list<StreetSegment> route, list<DeliveryCommand>& commands) const {
    commands.clear();
    StreetSegment prevSeg;

    for (auto it = route.begin(); it != route.end(); it++) {
        DeliveryCommand command;

        double angle = angleOfLine(*it);
        string dir = getDirection(angle);

        double dist = distanceEarthMiles(it->start, it->end);

        // For the first segment
        if (it == route.begin()) {
            command.initAsProceedCommand(dir, it->name, dist);
            commands.push_back(command);
            prevSeg = *it;
            continue;
        }

        // other segments

        // same street! keep proceeding
        if (it->name == prevSeg.name) {
            commands.rbegin()->increaseDistance(dist);
            prevSeg = *it;
            continue;
        }

        // we have a new street

        double turnAngle = angleBetween2Lines(prevSeg, *it);

        // street is a turn

        // left
        if (turnAngle < 180) {
            command.initAsTurnCommand("left", it->name);
        }
        else {
            // right
            command.initAsTurnCommand("right", it->name);
        }
        if (turnAngle > 1 && turnAngle < 359) commands.push_back(command);

        // if street is similar angle. Generate proceed with no turn.

        command.initAsProceedCommand(dir, it->name, dist);
        commands.push_back(command);
        prevSeg = *it;
    }

    return;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
