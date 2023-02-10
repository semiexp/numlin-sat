#pragma once

#include <optional>

#include "core/Constraint.h"
#include "core/Solver.h"

#include "Grid.h"

namespace numlin {

// Propagator for non-trivial Numberlink constraints.
class Propagator : public Glucose::Constraint {
public:
    Propagator(const Grid<int>& problem, const Grid<Glucose::Lit>& horizontal, const Grid<Glucose::Lit>& vertical);
    virtual ~Propagator() = default;

    bool initialize(Glucose::Solver& solver) override;
    bool propagate(Glucose::Solver& solver, Glucose::Lit p) override;
    void calcReason(Glucose::Solver& solver, Glucose::Lit p, Glucose::Lit extra,
                    Glucose::vec<Glucose::Lit>& out_reason) override;
    void undo(Glucose::Solver& solver, Glucose::Lit p) override;

private:
    std::optional<std::vector<Glucose::Lit>> Solve(Glucose::Solver& solver);
    void Dump(const Glucose::Solver& solver) const;

    Grid<int> problem_;
    Grid<Glucose::Lit> horizontal_, vertical_;
    std::vector<std::vector<Glucose::Lit>> reasons_;

    int height_;
    int width_;
};

}
