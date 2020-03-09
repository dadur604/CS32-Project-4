#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <iostream>
#include "ExpandableHashMap.h"
#include <unordered_set>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;

private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment*>> hashMap;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{

    ifstream is(mapFile);

    if (!is) {
        // Failed open
    }

    string line;
    while (getline(is, line)) {
        // Get the street name
        string name = line;

        // get the num of segments
        int numSeg;
        is >> numSeg;

        is.ignore(1000, '\n');
        
        for (int i = 0; i < numSeg; i++) {
            if (!getline(is, line)) {
                // Failed read
            }

            istringstream is_seg(line);

            string lat1, long1, lat2, long2;

            if (!(is_seg >> lat1 >> long1 >> lat2 >> long2)) {
                // Failed read
            }

            // Mapping

            GeoCoord start(lat1, long1);
            GeoCoord end(lat2, long2);

            vector<StreetSegment*>* connectedSegments;
            if (! (connectedSegments = hashMap.find(start))) {
                connectedSegments = &vector<StreetSegment*>();
                hashMap.associate(start, *connectedSegments);
                connectedSegments = hashMap.find(start);
            }

            // connectedSegments is the vector of Street Segments that maps to start

            vector<StreetSegment*>::iterator it;

            StreetSegment* segment = new StreetSegment(start, end, name);

            connectedSegments->push_back(segment);

            // Reverse

            if (!(connectedSegments = hashMap.find(end))) {
                connectedSegments = &vector<StreetSegment*>();
                hashMap.associate(end, *connectedSegments);
                connectedSegments = hashMap.find(end);
            }

            StreetSegment* reverseSegment = new StreetSegment(end, start, name);

            connectedSegments->push_back(reverseSegment);
        }
    }

    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment*>* foundSegments;
    if (! (foundSegments = hashMap.find(gc))) {
        return false;
    }

    segs.clear();
    for (auto it = foundSegments->begin(); it != foundSegments->end(); it++) {
        segs.push_back(**it);
    }

    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
