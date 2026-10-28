#ifndef SPACEDMERITERATOR_HPP
#define SPACEDMERITERATOR_HPP

#include <memory>
#include <jellyfish/mer_dna.hpp>
#include "fsh.hpp"

// Consumes raw masked-FASTA buffers without producing intermediate k-mer
// strings. Other formats retain their existing contiguous iterator behavior.
template<class SequencePool>
class SpacedMerIterator {
public:
    SpacedMerIterator(SequencePool& parser, const FSHPlan& plan, bool canonical)
        : job_(new typename SequencePool::job(parser)), current_(nullptr),
          plan_(plan), state_(plan), canonical_(canonical), filled_(0)
    {
        static_assert(sizeof(jellyfish::mer_dna::base_type) == sizeof(std::uint64_t),
                      "FSH requires Jellyfish's 64-bit mer_dna word type.");
        if (job_->is_empty()) job_.reset();
        else {
            current_ = (*job_)->start;
            advance();
        }
    }

    explicit operator bool() const { return static_cast<bool>(job_); }
    const jellyfish::mer_dna& operator*() const {
        return canonical_ && reverse_ < forward_ ? reverse_ : forward_;
    }
    SpacedMerIterator& operator++() {
        advance();
        return *this;
    }

private:
    std::unique_ptr<typename SequencePool::job> job_;
    const char* current_;
    const FSHPlan& plan_;
    FSHState state_;
    bool canonical_;
    unsigned int filled_;
    jellyfish::mer_dna forward_;
    jellyfish::mer_dna reverse_;

    void advance() {
        while (job_) {
            if ((*job_)->spaced) {
                while (static_cast<std::size_t>((*job_)->end - current_) >= plan_.span()) {
                    const bool valid = state_.next(current_++);
                    if (!valid) continue;
                    for (std::size_t word = 0; word < plan_.word_count(); ++word) {
                        forward_.word__(word) = state_.data()[word];
                    }
                    if (canonical_) {
                        for (std::size_t word = 0; word < plan_.word_count(); ++word) {
                            reverse_.word__(word) = state_.data()[word];
                        }
                        reverse_.reverse_complement();
                    }
                    return;
                }
            } else {
                while (current_ < (*job_)->end) {
                    const int code = jellyfish::mer_dna::code(*current_++);
                    if (code < 0) {
                        filled_ = 0;
                        continue;
                    }
                    forward_.shift_left(code);
                    if (canonical_) reverse_.shift_right(jellyfish::mer_dna::complement(code));
                    if (filled_ < jellyfish::mer_dna::k()) ++filled_;
                    if (filled_ == jellyfish::mer_dna::k()) return;
                }
            }
            job_->next();
            if (job_->is_empty()) {
                job_.reset();
                current_ = nullptr;
            } else {
                current_ = (*job_)->start;
                state_.reset();
                filled_ = 0;
            }
        }
    }
};

#endif // SPACEDMERITERATOR_HPP
