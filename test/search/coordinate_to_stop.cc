// make sure we get a new main function here
#define BOOST_TEST_MAIN
#include "fixtures.h"
#include <boost/test/unit_test.hpp>

#include "gtfs/dataset.hpp"
#include "gtfs/read_csv.hpp"
#include "gtfs/stop.hpp"

#include "geometric/bounding_box.hpp"
#include "geometric/coordinate.hpp"
#include "search/coordinate_to_stop.hpp"

using namespace transit;

BOOST_AUTO_TEST_CASE(lookup_lines_from_stops)
{
    // This needs to be replaced by a dedicated fixture (see
    // https://github.com/mapbox/directions-transit/issues/37)
    transit::gtfs::CSVDiscSource source(TRANSIT_THREE_LINES_EXAMPLE_FIXTURE);
    auto dataset = transit::gtfs::readCSV(source);

    auto make_coordinate_lookup = [&]() {
        std::vector<std::pair<geometric::WGS84Coordinate, gtfs::StopID>> data;
        std::for_each(dataset.stops.begin(), dataset.stops.end(), [&](auto const &element) {
            if (!element.location_type || *element.location_type == gtfs::LocationType::STOP)
                data.push_back(std::make_pair(element.location, element.id));
        });
        return search::CoordinateToStop(data);
    };

    auto coordinate_lookup = make_coordinate_lookup();

    geometric::WGS84Coordinate coordinate(
        geometric::makeLatLonFromDouble<geometric::FixedLongitude>(0.01001),
        geometric::makeLatLonFromDouble<geometric::FixedLatitude>(0.0001));

    auto closest_stop = coordinate_lookup.nearest(coordinate);
    BOOST_CHECK_EQUAL(closest_stop, gtfs::StopID{0});

    geometric::WGS84Coordinate zero;
    geometric::WGS84BoundingBox bbox(zero, coordinate);

    auto all = coordinate_lookup.all(bbox);
    BOOST_CHECK(all.size() == 1);
    BOOST_CHECK_EQUAL(all.front(), gtfs::StopID{0});
}