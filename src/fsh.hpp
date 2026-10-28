#ifndef FSH_HPP
#define FSH_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/** 
FSH Plan :
Accroding to spaced seed, calculate witch position's ATCG also including in next
spaced k-mer, need to shift how many positions.

FSH State :
Record previous counting k-mers, and follow the plan to count next k-mer.
**/

class FSHPlan {
public:
    FSHPlan(const std::string& seed, std::size_t expected_weight);
    std::size_t span() const { return span_; }
    std::size_t weight() const { return weight_; }
    std::size_t word_count() const { return word_count_; }

private:
    struct Position {
        std::size_t offset;
        std::size_t word;
        unsigned int shift;
    };

    struct Step {
        std::size_t lag = 0;
        std::size_t word_shift = 0;
        unsigned int bit_shift = 0;
        std::vector<std::uint64_t> keep;
        std::vector<Position> insert;
    };

    std::size_t span_;
    std::size_t weight_;
    std::size_t word_count_;
    std::size_t history_size_;
    std::vector<Step> steps_;
    std::vector<std::size_t> best_lag_;
    friend class FSHState;
};


class FSHState {
public:
    explicit FSHState(const FSHPlan& plan);
    void reset();
    bool next(const char* window);
    const std::uint64_t* data() const;

private:
    const FSHPlan& plan_;
    std::size_t position_;
    std::size_t current_;
    std::vector<std::uint64_t> hashes_;
    std::vector<std::uint64_t> undefined_;
};

#endif // FSH_HPP
