#include "timetable/departure_table_factory.hpp"
#include "timetable/exceptions.hpp"

#include <algorithm>

#include <boost/numeric/conversion/cast.hpp>

namespace transit
{
namespace timetable
{

namespace
{

template <typename iterator_type>
void check_input_validity(const iterator_type begin, const iterator_type end)
{
    if (!std::distance(begin, end))
        throw InvalidInputError("Frequency factory called without frequencies.");

    if (std::any_of(begin, end, [begin](auto const &frequency) {
            return frequency.trip_id != begin->trip_id;
        }))
    {
        throw InvalidInputError(
            "Frequencies need to represent the same trip for departur table construction.");
    }
}

} // namespace

DepartureTable DepartureTableFactory::produce(std::vector<gtfs::Frequency>::iterator begin,
                                              std::vector<gtfs::Frequency>::iterator end)
{
    DepartureTable table;
    check_input_validity(begin, end);

    table.departures.reserve(std::distance(begin, end));

    const auto to_departure = [](auto const &frequency) -> DepartureTable::Departure {
        return {
            frequency.begin, frequency.end, boost::numeric_cast<std::uint32_t>(frequency.headway)};
    };

    std::transform(begin, end, std::back_inserter(table.departures), to_departure);

    table._trip_id = begin->trip_id;

    std::sort(table.departures.begin(), table.departures.end());
    return table;
}

DepartureTable DepartureTableFactory::produce(std::vector<gtfs::StopTime>::iterator begin,
                                              std::vector<gtfs::StopTime>::iterator end)
{
    DepartureTable table;
    // at some time frequency / stop time might need different checks, right now we are fine doing
    // it this way and miss-using the input validity template
    check_input_validity(begin, end);

    table._trip_id = begin->trip_id;
    table.departures.push_back({begin->departure, begin->departure, 0u});
    return table;
}

} // namespace timetable
} // namespace transit
