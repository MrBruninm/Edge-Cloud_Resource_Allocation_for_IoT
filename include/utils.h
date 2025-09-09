#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace utils {

    //=========================================================================
    // Type Conversion Utilities
    //=========================================================================

    /**
     * @brief Converts a value of any type to its string representation.
     * @details This template function uses `std::ostringstream` for conversion. It
     * includes a compile-time check (`if constexpr`) to apply fixed-point
     * notation with a specified precision for floating-point types, ensuring
     * consistent and predictable formatting for numbers.
     *
     * @tparam T The data type of the value to be converted.
     * @param[in] value The value to convert.
     * @param[in] precision The number of decimal places for floating-point types.
     * This parameter is ignored for non-floating-point types.
     * @return A `std::string` representation of the input value.
     */
    template <typename T>
    inline std::string toString(const T& value, int precision = 6) {
        std::ostringstream out;
        if constexpr (std::is_floating_point_v<T>) {
            out << std::fixed << std::setprecision(precision);
        }
        out << value;
        return out.str();
    }

    /**
     * @brief Calculates a percentage and converts it to a formatted string.
     * @details This function computes `(numerator / denominator) * 100.0` and formats
     * the result using the `toString` utility. It includes a safety check to
     * handle cases where the denominator is zero, returning "0.0" to avoid
     * division-by-zero errors.
     *
     * @param[in] numerator The value representing the part of the total.
     * @param[in] denominator The value representing the total.
     * @param[in] precision The number of decimal places for the resulting percentage string.
     * @return A string representation of the calculated percentage.
     */
    inline std::string toPercentageString(double numerator, double denominator, int precision = 4) {
        if (denominator == 0) {
            return toString(0.0, precision);
        }
        double percentage = (numerator / denominator) * 100.0;
        return toString(percentage, precision);
    }

    //=========================================================================
    // Randomness Utilities
    //=========================================================================

    /**
     * @brief Provides a singleton instance of a high-quality random number engine.
     * @details This function returns a reference to a static `std::mt19937` engine.
     * The engine is seeded only once during the first call using a
     * non-deterministic value from `std::random_device`. This ensures that all
     * random number generation throughout the application shares the same
     * seeded engine for efficiency. Note: This implementation is not
     * guaranteed to be thread-safe without external locking.
     *
     * @return A reference to the static `mt19937` random engine.
     */
    inline std::mt19937& getEngine() {
        static std::random_device rd;
        static std::mt19937 engine(rd());
        return engine;
    }

    /**
     * @brief Generates a random number within a specified inclusive interval `[min, max]`.
     * @details This function is templated to work for both integral and floating-point
     * types. It uses `if constexpr` to select the correct uniform distribution
     * (`std::uniform_int_distribution` or `std::uniform_real_distribution`)
     * at compile-time.
     *
     * @tparam T The numeric type (e.g., int, double). Must be an arithmetic type.
     * @param[in] min The lower bound of the interval (inclusive).
     * @param[in] max The upper bound of the interval (inclusive).
     * @return A random number of type T within the specified range.
     * @throws std::invalid_argument if `min` is greater than `max`.
     */
    template<typename T>
    inline T randomNumber(T min, T max) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type.");
        if (min > max) {
            throw std::invalid_argument("Error in randomNumber: min cannot be greater than max.");
        }
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> distribution(min, max);
            return distribution(getEngine());
        } else {
            std::uniform_real_distribution<T> distribution(min, max);
            return distribution(getEngine());
        }
    }

    /**
     * @brief Creates a vector of unique integers in random order within an inclusive range `[min, max]`.
     * @details This function first creates a vector and populates it with a sequential
     * range of numbers from `min` to `max` using `std::iota`. It then shuffles
     * the vector in-place using `std::shuffle` and the shared random engine.
     *
     * @tparam T The integral type for the range. Must be an integral type.
     * @param[in] min The lower bound of the range (inclusive).
     * @param[in] max The upper bound of the range (inclusive).
     * @return A `std::vector<T>` containing all integers from `min` to `max`, randomly shuffled.
     * @throws std::invalid_argument if `min` is greater than `max`.
     */
    template<typename T>
    inline std::vector<T> shuffledRange(T min, T max) {
        static_assert(std::is_integral_v<T>, "shuffledRange requires an integral type.");
        if (min > max) {
            throw std::invalid_argument("Error in shuffledRange: min cannot be greater than max.");
        }
        std::vector<T> numbers(max - min + 1);
        std::iota(numbers.begin(), numbers.end(), min);
        std::shuffle(numbers.begin(), numbers.end(), getEngine());
        return numbers;
    }

    //=========================================================================
    // Sorting Utilities
    //=========================================================================
    
    namespace details {
        /**
         * @brief Internal implementation for sorting indices. Do not call directly.
         */
        template <bool Ascending, typename T, typename AttributeType, AttributeType T::* AttributePtr>
        void sort_indices_impl(const std::vector<T>& entities, std::vector<int>& idxs) {
            std::sort(idxs.begin(), idxs.end(), [&](int i, int j) {
                const auto& attr_i = entities.at(i).*AttributePtr;
                const auto& attr_j = entities.at(j).*AttributePtr;

                if (attr_i != attr_j) {
                    if constexpr (Ascending) {
                        return attr_i < attr_j; // Ascending order
                    } else {
                        return attr_i > attr_j; // Descending order
                    }
                }
                return i < j; // Tie-breaker
            });
        }
    }

    /**
     * @brief [OVERLOAD 1] Sorts the indices of all entities (from 1 to N-1) by a specific attribute.
     * @details Returns a new vector containing the sorted indices of entities without modifying
     * the original container. A tie-breaking rule sorts by the original index value to
     * ensure a deterministic order when attribute values are equal.
     * @tparam Ascending If `true`, sorts in ascending order; if `false`, sorts descending.
     * @tparam T The type of the entity struct/class in the vector.
     * @tparam AttributeType The data type of the member attribute to sort by.
     * @tparam AttributePtr A pointer-to-member specifying the attribute for comparison.
     * @param[in] entities The constant vector of entities from which attributes are read.
     * @return A new `std::vector<int>` containing the sorted indices.
     */
    template <bool Ascending, typename T, typename AttributeType, AttributeType T::* AttributePtr>
    inline std::vector<int> sortEntities(const std::vector<T>& entities) {
        if (entities.size() <= 1) return {};
        std::vector<int> idxs(entities.size() - 1);
        std::iota(idxs.begin(), idxs.end(), 1); // Assumes 1-based indexing
        details::sort_indices_impl<Ascending, T, AttributeType, AttributePtr>(entities, idxs);
        return idxs;
    }

    /**
     * @brief [OVERLOAD 2] Sorts a given subset of entity indices by a specific attribute.
     * @details Similar to the first overload, but operates on a provided subset of indices.
     * This is useful for sorting filtered lists. The tie-breaking rule still applies.
     * @tparam Ascending If `true`, sorts in ascending order; if `false`, sorts descending.
     * @tparam T The type of the entity struct/class in the vector.
     * @tparam AttributeType The data type of the member attribute to sort by.
     * @tparam AttributePtr A pointer-to-member specifying the attribute for comparison.
     * @param[in] entities The constant vector of entities.
     * @param[in] indices_to_sort A vector of integers representing the subset of indices to sort.
     * @return A new `std::vector<int>` containing the sorted indices from the subset.
     */
    template <bool Ascending, typename T, typename AttributeType, AttributeType T::* AttributePtr>
    inline std::vector<int> sortEntities(const std::vector<T>& entities, const std::vector<int>& indices_to_sort) {
        if (entities.empty() || indices_to_sort.empty()) return {};
        std::vector<int> idxs = indices_to_sort; // Create a copy to sort
        details::sort_indices_impl<Ascending, T, AttributeType, AttributePtr>(entities, idxs);
        return idxs;
    }

    //=========================================================================
    // Geographic Utilities
    //=========================================================================

    constexpr long double EARTH_RADIUS_KM = 6371.0088L;
    constexpr long double PI_L = 3.141592653589793238462643383279502884L;

    /**
     * @brief Calculates the distance between two points using the Haversine formula.
     * @note This is a helper function that expects all latitude and longitude
     * inputs to be in **radians**. For calculations with degrees, use the primary
     * `calculateDistance` function.
     *
     * @param[in] lat1_rad Latitude of point 1 (radians).
     * @param[in] lon1_rad Longitude of point 1 (radians).
     * @param[in] lat2_rad Latitude of point 2 (radians).
     * @param[in] lon2_rad Longitude of point 2 (radians).
     * @return The great-circle distance in kilometers.
     */
    inline double haversineDistance(long double lat1_rad, long double lon1_rad, long double lat2_rad, long double lon2_rad) {
        const long double dlat = lat2_rad - lat1_rad;
        const long double dlon = lon2_rad - lon1_rad;
        
        const long double a = pow(sin(dlat / 2.0L), 2) + cos(lat1_rad) * cos(lat2_rad) * pow(sin(dlon / 2.0L), 2);
        const long double c = 2.0L * atan2(sqrt(a), sqrt(1.0L - a));
        
        return static_cast<double>(EARTH_RADIUS_KM * c);
    }

    /**
     * @brief Calculates the distance between two geographic coordinates given in degrees.
     * @details This function balances performance and precision. It first uses a fast
     * equirectangular approximation. If the estimated distance is below a small
     * threshold (1.5 km), this approximation is returned. For longer distances,
     * it falls back to the more accurate but computationally intensive Haversine
     * formula to ensure precision.
     *
     * @param[in] lat1_deg Latitude of point 1 (in degrees).
     * @param[in] lon1_deg Longitude of point 1 (in degrees).
     * @param[in] lat2_deg Latitude of point 2 (in degrees).
     * @param[in] lon2_deg Longitude of point 2 (in degrees).
     * @return The distance between the two points in kilometers.
     */
    inline double calculateDistance(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg) {
        const long double lat1_rad = lat1_deg * (PI_L / 180.0L);
        const long double lon1_rad = lon1_deg * (PI_L / 180.0L);
        const long double lat2_rad = lat2_deg * (PI_L / 180.0L);
        const long double lon2_rad = lon2_deg * (PI_L / 180.0L);

        const double threshold_km = 1.5;
        const long double x = (lon2_rad - lon1_rad) * cos((lat1_rad + lat2_rad) / 2.0L);
        const long double y = (lat2_rad - lat1_rad);
        const double planar_distance = static_cast<double>(sqrt(x * x + y * y) * EARTH_RADIUS_KM);
        
        if (planar_distance < threshold_km) {
            return planar_distance;
        }
        
        return haversineDistance(lat1_rad, lon1_rad, lat2_rad, lon2_rad);
    }
}