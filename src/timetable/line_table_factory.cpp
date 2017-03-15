#include "timetable/line_table_factory.hpp"
#include "timetable/departure_table.hpp"
#include "timetable/duration_table_factory.hpp"
#include "timetable/stop_table_factory.hpp"

#include "algorithm/ranges.hpp"

#include "boost/assert.hpp"

namespace transit
{
namespace timetable
{

template <typename iterator_type>
void interpolate_times(iterator_type begin, const iterator_type end)
{
    if (std::none_of(
            begin, end, [](auto const &stop_time) { return stop_time.arrival.interpolated; }))
        return;

    BOOST_ASSERT(!begin->arrival.interpolated);
    BOOST_ASSERT(!(end - 1)->arrival.interpolated);

    auto const interpolate = [](std::pair<iterator_type, iterator_type> range) {
        const auto delta_t_seconds_total = range.second->arrival - (range.first - 1)->arrival;
        const auto delta_t_seconds =
            delta_t_seconds_total / (std::distance(range.first, range.second) + 1);
        auto current_time = (range.first - 1)->arrival;
        auto const set_new_time = [&current_time, delta_t_seconds](auto stop_time) {
            current_time = current_time + delta_t_seconds;
            stop_time.arrival = current_time;
            return stop_time;
        };
        // in place transform
        std::transform(range.first, range.second, range.first, set_new_time);
    };

    // interpolate between two non-interpolated times
    algorithm::on_ranges(begin,
                         end,
                         [](const auto &stop_time) { return stop_time.arrival.interpolated; },
                         interpolate);
}

std::vector<LineTable> LineTableFactory::produce(std::vector<gtfs::StopTime>::iterator const begin,
                                                 std::vector<gtfs::StopTime>::iterator const end)
{
    BOOST_ASSERT(std::distance(begin, end) != 0);
    std::vector<LineTable> line_tables;

    auto const by_trip_id = [](auto const &lhs, auto const &rhs) {
        return lhs.trip_id < rhs.trip_id;
    };

    using InputIter = typename std::vector<gtfs::StopTime>::iterator;
    using InputIterRange = std::pair<InputIter, InputIter>;

    // compute all departures for the line
    const auto output_inserter = [&line_tables](InputIterRange const range) {
        interpolate_times(range.first, range.second);

        auto stop_table = StopTableFactory::produce(range.first, range.second);

        auto const line_index = [&line_tables, stop_table]() {
            auto const itr =
                std::find_if(line_tables.begin(), line_tables.end(), [&](auto const &line) {
                    return line.stop_table == stop_table;
                });

            if (itr == line_tables.end())
            {
                std::cout << "New Line Table:";
                for (auto s : stop_table.list())
                    std::cout << " " << s;
                std::cout << std::endl;
                line_tables.push_back(LineTable());
                line_tables.back().stop_table = stop_table;
                return line_tables.size() - 1;
            }
            else
            {
                std::cout << "Found line table at index: "
                          << std::distance(line_tables.begin(), itr) << std::endl;
                return static_cast<std::size_t>(std::distance(line_tables.begin(), itr));
            }
        }();

        auto duration_table = DurationTableFactory::produce(range.first, range.second);
        auto const duration_index = [&line_tables, &duration_table, line_index]() {
            auto const itr = std::find(line_tables[line_index].duration_tables.begin(),
                                       line_tables[line_index].duration_tables.end(),
                                       duration_table);
            if (itr == line_tables[line_index].duration_tables.end())
            {
                std::cout << "New Duration Table:";
                for (auto s : duration_table.list(0))
                    std::cout << " " << s;
                std::cout << std::endl;
                line_tables[line_index].duration_tables.push_back(duration_table);
                return line_tables[line_index].duration_tables.size() - 1;
            }
            else
            {
                std::cout << "Found duration table at index: "
                          << std::distance(line_tables[line_index].duration_tables.begin(), itr)
                          << std::endl;
                return static_cast<std::size_t>(
                    std::distance(line_tables[line_index].duration_tables.begin(), itr));
            }
        }();

        line_tables[line_index].departures.departures.push_back(
            {range.first->departure, range.first->departure, 0, duration_index});
    };

    std::cout << "New Line Group" << std::endl;
    algorithm::by_equal_ranges(begin, end, by_trip_id, output_inserter);

    // sort departures
    for (auto &line_table : line_tables)
        std::sort(line_table.departures.departures.begin(),
                  line_table.departures.departures.end(),
                  [](auto const &lhs, auto const &rhs) { return lhs.begin < rhs.begin; });

    return line_tables;
}

} // namespace timetable
} // namespace transit
