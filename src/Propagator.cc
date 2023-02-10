#include "Propagator.h"

#include <cassert>

namespace numlin {

Propagator::Propagator(const Grid<int>& problem, const Grid<Glucose::Lit>& horizontal, const Grid<Glucose::Lit>& vertical)
    : problem_(problem), horizontal_(horizontal), vertical_(vertical), height_(problem_.height()), width_(problem_.width()) {
    assert(horizontal_.height() == height_);
    assert(horizontal_.width() == width_ - 1);
    assert(vertical_.height() == height_ - 1);
    assert(vertical_.width() == width_);
}

bool Propagator::initialize(Glucose::Solver& solver) {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_ - 1; ++x) {
            solver.addWatch(horizontal_.at(y, x), this);
            solver.addWatch(~horizontal_.at(y, x), this);
        }
    }
    for (int y = 0; y < height_ - 1; ++y) {
        for (int x = 0; x < width_; ++x) {
            solver.addWatch(vertical_.at(y, x), this);
            solver.addWatch(~vertical_.at(y, x), this);
        }
    }

    // suppose we have nothing to propagate at this stage

    return true;
}

bool Propagator::propagate(Glucose::Solver& solver, Glucose::Lit p) {
    solver.registerUndo(var(p), this);

    if (num_pending_propagation() > 0) {
        reasons_.push_back({});
        return true;
    }

    auto res = Solve(solver);
    if (res) {
        reasons_.push_back(*res);
        return false;
    } else {
        reasons_.push_back({});
        return true;
    }
}

void Propagator::calcReason(Glucose::Solver& solver, Glucose::Lit p, Glucose::Lit extra, Glucose::vec<Glucose::Lit>& out_reason) {
    assert(!reasons_.empty());
    assert(p == Glucose::lit_Undef);

    for (auto& lit : reasons_.back()) {
        out_reason.push(lit);
    }

    if (extra != Glucose::lit_Undef) {
        out_reason.push(extra);
    }
}

void Propagator::undo(Glucose::Solver& solver, Glucose::Lit p) {
    reasons_.pop_back();
}

std::optional<std::vector<Glucose::Lit>> Propagator::Solve(Glucose::Solver& solver) {
    std::vector<Glucose::Lit> path;
    Dump(solver);

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (problem_.at(y, x) == -1) {
                continue;
            }

            path.clear();
            int cy = y, cx = x;
            int ly = -1, lx = -1;
            for (;;) {
                if (cy > 0 && ly != cy - 1 && solver.value(vertical_.at(cy - 1, cx)) == l_True) {
                    path.push_back(vertical_.at(cy - 1, cx));
                    ly = cy; lx = cx;
                    cy -= 1;
                    continue;
                }
                if (cy < height_ - 1 && ly != cy + 1 && solver.value(vertical_.at(cy, cx)) == l_True) {
                    path.push_back(vertical_.at(cy, cx));
                    ly = cy; lx = cx;
                    cy += 1;
                    continue;
                }
                if (cx > 0 && lx != cx - 1 && solver.value(horizontal_.at(cy, cx - 1)) == l_True) {
                    path.push_back(horizontal_.at(cy, cx - 1));
                    ly = cy; lx = cx;
                    cx -= 1;
                    continue;
                }
                if (cx < width_ - 1 && lx != cx + 1 && solver.value(horizontal_.at(cy, cx)) == l_True) {
                    path.push_back(horizontal_.at(cy, cx));
                    ly = cy; lx = cx;
                    cx += 1;
                    continue;
                }
                break;
            }

            if (problem_.at(cy, cx) != -1 && problem_.at(cy, cx) != problem_.at(y, x)) {
                // connecting different number
                return path;
            }
        }
    }

    for (int y = 0; y < height_ - 1; ++y) {
        int last_pt = -1;
        for (int x = 0; x < width_; ++x) {
            if (last_pt >= 0) {
                if (!(solver.value(horizontal_.at(y, x - 1)) == l_False && solver.value(horizontal_.at(y + 1, x - 1)) == l_True)) {
                    last_pt = -1;
                }
            }

            if (solver.value(vertical_.at(y, x)) == l_True) {
                if (last_pt != -1) {
                    std::vector<Glucose::Lit> reason;
                    reason.push_back(vertical_.at(y, last_pt));
                    reason.push_back(vertical_.at(y, x));
                    for (int i = last_pt; i < x; ++i) {
                        reason.push_back(~horizontal_.at(y, i));
                        reason.push_back(horizontal_.at(y + 1, i));
                    }
                    return reason;
                }
                last_pt = x;
            } else if (problem_.at(y, x) >= 0 || problem_.at(y + 1, x) >= 0) {
                last_pt = -1;
            }
        }
    }
    // TODO: disallow cycles

    return std::nullopt;
}

void Propagator::Dump(const Glucose::Solver& solver) const {
    for (int y = 0; y < height_ * 2 - 1; ++y) {
        for (int x = 0; x < width_ * 2 - 1; ++x) {
            if (y % 2 == 0 && x % 2 == 0) {
                int c = problem_.at(y / 2, x / 2);
                if (c == -1) {
                    printf("+");
                } else if (c >= 0 && c <= 9) {
                    printf("%c", c + '0');
                } else {
                    printf("%c", c - 10 + 'A');
                }
            } else if (y % 2 == 0 && x % 2 == 1) {
                Glucose::lbool state = solver.value(horizontal_.at(y / 2, x / 2));
                if (state == l_True) {
                    printf("-");
                } else if (state == l_False) {
                    printf("x");
                } else {
                    printf(" ");
                }
            } else if (y % 2 == 1 && x % 2 == 0) {
                Glucose::lbool state = solver.value(vertical_.at(y / 2, x / 2));
                if (state == l_True) {
                    printf("|");
                } else if (state == l_False) {
                    printf("x");
                } else {
                    printf(" ");
                }
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("==================================\n");
}

}
