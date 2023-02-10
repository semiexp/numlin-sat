#include "Solver.h"

#include <cstdio>
#include <memory>
#include <vector>

#include "core/Solver.h"

#include "Propagator.h"

namespace numlin {

void Solve(const Grid<int>& problem) {
    int height = problem.height();
    int width = problem.width();

    Glucose::Solver solver;

    Grid<Glucose::Lit> horizontal(height, width - 1);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            horizontal.at(y, x) = Glucose::mkLit(solver.newVar());
        }
    }
    Grid<Glucose::Lit> vertical(height - 1, width);
    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width; ++x) {
            vertical.at(y, x) = Glucose::mkLit(solver.newVar());
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::vector<Glucose::Lit> adj;
            
            if (y > 0) {
                adj.push_back(vertical.at(y - 1, x));
            }
            if (y < height - 1) {
                adj.push_back(vertical.at(y, x));
            }
            if (x > 0) {
                adj.push_back(horizontal.at(y, x - 1));
            }
            if (x < width - 1) {
                adj.push_back(horizontal.at(y, x));
            }

            if (problem.at(y, x) >= 0) {
                // has clue: degree == 1
                Glucose::vec<Glucose::Lit> clause;
                for (int i = 0; i < adj.size(); ++i) {
                    clause.push(adj[i]);
                }
                solver.addClause(clause);
                for (int i = 0; i < adj.size(); ++i) {
                    for (int j = i + 1; j < adj.size(); ++j) {
                        solver.addClause(~adj[i], ~adj[j]);
                    }
                }
            } else {
                // no clue: degree == 0 or 2

                // degree != 1
                for (int i = 0; i < adj.size(); ++i) {
                    Glucose::vec<Glucose::Lit> clause;
                    for (int j = 0; j < adj.size(); ++j) {
                        clause.push(i == j ? ~adj[j] : adj[j]);
                    }
                    solver.addClause(clause);
                }

                // degree < 3
                for (int i = 0; i < adj.size(); ++i) {
                    for (int j = i + 1; j < adj.size(); ++j) {
                        for (int k = j + 1; k < adj.size(); ++k) {
                            solver.addClause(~adj[i], ~adj[j], ~adj[k]);
                        }
                    }
                }
            }
        }
    }

    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            Glucose::Lit nb[] = {
                vertical.at(y, x),
                vertical.at(y, x + 1),
                horizontal.at(y, x),
                horizontal.at(y + 1, x),
            };
            for (int i = 0; i < 4; ++i) {
                solver.addClause(~nb[i], ~nb[(i + 1) % 4], ~nb[(i + 2) % 4]);
            }
        }
    }

    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            if (problem.at(y + 1, x + 1) < 0) {
                Glucose::vec<Glucose::Lit> clause;
                clause.push(~vertical.at(y, x));
                clause.push(~horizontal.at(y, x));
                if (x < width - 2) {
                    clause.push(horizontal.at(y + 1, x + 1));
                }
                if (y < height - 2) {
                    clause.push(vertical.at(y + 1, x + 1));
                }
                solver.addClause(clause);
            }
            if (problem.at(y + 1, x) < 0) {
                Glucose::vec<Glucose::Lit> clause;
                clause.push(~vertical.at(y, x + 1));
                clause.push(~horizontal.at(y, x));
                if (x > 0) {
                    clause.push(horizontal.at(y + 1, x - 1));
                }
                if (y < height - 2) {
                    clause.push(vertical.at(y + 1, x));
                }
                solver.addClause(clause);
            }
        }
    }
    solver.addConstraint(std::make_unique<Propagator>(problem, horizontal, vertical));

    for (;;) {
        if (!solver.solve()) {
            break;
        }

        Glucose::vec<Glucose::Lit> refutation;

        Grid<int> ans_horizontal(height, width - 1);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width - 1; ++x) {
                bool c = solver.modelValue(horizontal.at(y, x)) == l_True;
                ans_horizontal.at(y, x) = c;
                if (c) {
                    refutation.push(~horizontal.at(y, x));
                } else {
                    refutation.push(horizontal.at(y, x));
                }
            }
        }

        Grid<int> ans_vertical(height - 1, width);
        for (int y = 0; y < height - 1; ++y) {
            for (int x = 0; x < width; ++x) {
                bool c = solver.modelValue(vertical.at(y, x)) == l_True;
                ans_vertical.at(y, x) = c;
                if (c) {
                    refutation.push(~vertical.at(y, x));
                } else {
                    refutation.push(vertical.at(y, x));
                }
            }
        }
        solver.addClause(refutation);

        puts("found");

        for (int y = 0; y < height * 2 - 1; ++y) {
            for (int x = 0; x < width * 2 - 1; ++x) {
                if (y % 2 == 0 && x % 2 == 0) {
                    printf("+");
                } else if (y % 2 == 1 && x % 2 == 0) {
                    printf("%c", ans_vertical.at(y / 2, x / 2) ? '|' : ' ');
                } else if (y % 2 == 0 && x % 2 == 1) {
                    printf("%c", ans_horizontal.at(y / 2, x / 2) ? '-' : ' ');
                } else {
                    printf(" ");
                }
            }
            puts("");
        }
    }
}

}
