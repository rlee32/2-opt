#include "DistanceTable.h"
#include "Optimizer.h"
#include "PointSequence.h"
#include "fileio/PointSet.h"
#include "fileio/Tour.h"
#include "primitives.h"
#include "verify.h"

#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
    // Read points.
    if (argc < 2)
    {
        std::cout << "Arguments: point_set_file_path optional_tour_file_path" << std::endl;
        return 0;
    }
    fileio::PointSet point_set(argv[1]);
    // Initial tour.
    std::vector<primitives::point_id_t> tour;
    if (argc > 2)
    {
        fileio::Tour initial_tour(argv[2]);
        tour = initial_tour.point_ids();
    }
    else
    {
        for (primitives::point_id_t i{0}; i < point_set.count(); ++i)
        {
            tour.push_back(i);
        }
    }
    // Initialize segments.
    PointSequence point_sequence(tour);
    const auto& next = point_sequence.next();
    std::unordered_set<Segment, Segment::Hash> segments;
    DistanceTable dt(point_set.x(), point_set.y());
    for (auto id : tour)
    {
        auto length = dt.lookup_length(id, next[id]);
        segments.insert({id, next[id], length});
    }
    // Hill climbing optimization loop.
    Optimizer optimizer(dt);
    auto prev_length = verify::tour_length(segments, dt);
    std::cout << "Initial tour length: " << prev_length << std::endl;
    int iteration{0};
    primitives::length_t improvement{1};
    while (improvement > 0)
    {
        optimizer.find_best(segments);
        std::cout << optimizer << std::endl;
        point_sequence.new_tour(segments, optimizer.best().segments, optimizer.best().new_segments);
        if (verify::valid_cycle(segments))
        {
            std::cout << "Tour is still a valid cycle." << std::endl;
        }
        else
        {
            std::cout << "ERROR: tour has become invalid!" << std::endl;
            break;
        }
        auto current_length = verify::tour_length(segments, dt);
        improvement = prev_length - current_length;
        std::cout << "Iteration " << iteration
            << " final tour length: " << current_length
            << " (step improvement: " << improvement << ")"
            << std::endl;
        ++iteration;
        prev_length = current_length;
    }
    std::cout << "Local optimum reached." << std::endl;
    return 0;
}
