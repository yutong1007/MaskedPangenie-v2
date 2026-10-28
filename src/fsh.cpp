#include "fsh.hpp"

#include <algorithm>
#include <stdexcept>

namespace {
int encode_base(char base) {
    switch (base) {
        case 'A': case 'a': return 0;
        case 'C': case 'c': return 1;
        case 'G': case 'g': return 2;
        case 'T': case 't': return 3;
        default: return -1;
    }
}

std::uint64_t shifted_word(const std::uint64_t* source, std::size_t word,
                           std::size_t word_shift, unsigned int bit_shift) {
    if (word < word_shift) return 0;
    const std::size_t index = word - word_shift;
    std::uint64_t result = source[index] << bit_shift;
    if (bit_shift != 0 && index != 0) {
        result |= source[index - 1] >> (64 - bit_shift);
    }
    return result;
}
}

FSHPlan::FSHPlan(const std::string& seed, std::size_t expected_weight)
    : span_(seed.size()), weight_(0), word_count_(0), history_size_(1)
{
    if (seed.empty()) throw std::invalid_argument("FSH: seed must not be empty.");

    std::vector<std::size_t> rank(span_ + 1, 0);
    for (std::size_t i = 0; i < span_; ++i) {
        if (seed[i] != '0' && seed[i] != '1') {
            throw std::invalid_argument("FSH: seed must contain only 0 and 1.");
        }
        rank[i + 1] = rank[i] + (seed[i] == '1');
    }
    weight_ = rank.back();
    if (weight_ == 0 || weight_ != expected_weight) {
        throw std::invalid_argument("FSH: seed weight must equal the k-mer size.");
    }
    word_count_ = (weight_ + 31) / 32;

    std::vector<Position> positions;
    for (std::size_t i = 0; i < span_; ++i) {
        if (seed[i] == '1') {
            const std::size_t bit = 2 * (weight_ - 1 - rank[i]);
            positions.push_back(Position{i, bit / 64,
                                         static_cast<unsigned int>(bit % 64)});
        }
    }

    steps_.resize(span_);
    best_lag_.assign(span_, 0);
    steps_[0].keep.assign(word_count_, 0);
    steps_[0].insert = positions;
    std::size_t best_kept = 0;
    for (std::size_t lag = 1; lag < span_; ++lag) {
        Step& step = steps_[lag];
        step.lag = lag;
        const std::size_t shift = 2 * rank[lag];
        step.word_shift = shift / 64;
        step.bit_shift = static_cast<unsigned int>(shift % 64);
        step.keep.assign(word_count_, 0);
        std::size_t kept = 0;
        for (const Position& position : positions) {
            const std::size_t offset = position.offset;
            // FSH C_lag: the same source base must also occupy the same
            // packed position after the rank-based shift.
            if (lag < span_ - offset && seed[offset + lag] == '1' &&
                rank[offset + lag] - rank[offset] == rank[lag]) {
                step.keep[position.word] |= std::uint64_t(3) << position.shift;
                ++kept;
            } else {
                step.insert.push_back(position);
            }
        }
        best_lag_[lag] = best_lag_[lag - 1];
        if (kept > best_kept) {
            best_kept = kept;
            best_lag_[lag] = lag;
        }
        history_size_ = std::max(history_size_, best_lag_[lag] + 1);
    }
}

FSHState::FSHState(const FSHPlan& plan)
    : plan_(plan), position_(0), current_(0),
      hashes_(plan.history_size_ * plan.word_count_, 0),
      undefined_(plan.history_size_ * plan.word_count_, 0)
{}

void FSHState::reset() {
    position_ = 0;
    current_ = 0;
}

bool FSHState::next(const char* window) {
    const FSHPlan::Step& step = plan_.steps_[plan_.best_lag_[
        std::min(position_, plan_.span_ - 1)]];
    current_ = position_ % plan_.history_size_;
    std::uint64_t* hash = hashes_.data() + current_ * plan_.word_count_;
    std::uint64_t* undefined = undefined_.data() + current_ * plan_.word_count_;
    std::uint64_t invalid = 0;

    if (step.lag != 0) {
        const std::size_t previous =
            ((position_ - step.lag) % plan_.history_size_) * plan_.word_count_;
        for (std::size_t word = 0; word < plan_.word_count_; ++word) {
            hash[word] = shifted_word(hashes_.data() + previous, word,
                                      step.word_shift, step.bit_shift) & step.keep[word];
            undefined[word] = shifted_word(undefined_.data() + previous, word,
                                           step.word_shift, step.bit_shift) & step.keep[word];
            invalid |= undefined[word];
        }
    } else {
        std::fill(hash, hash + plan_.word_count_, 0);
        std::fill(undefined, undefined + plan_.word_count_, 0);
    }

    for (const FSHPlan::Position& position : step.insert) {
        const int code = encode_base(window[position.offset]);
        if (code < 0) {
            const std::uint64_t mask = std::uint64_t(3) << position.shift;
            undefined[position.word] |= mask;
            invalid |= mask;
        } else {
            hash[position.word] |= std::uint64_t(code) << position.shift;
        }
    }
    ++position_;
    return invalid == 0;
}

const std::uint64_t* FSHState::data() const {
    return hashes_.data() + current_ * plan_.word_count_;
}
