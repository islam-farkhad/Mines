#include "minesweeper.h"

#include <random>
#include <queue>
#include <time.h>

Minesweeper::Minesweeper(size_t width, size_t height)
    : field_width_(width), field_height_(height), field_(height, std::vector<CellInfo>(width)) {

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            field_[i][j].x = j;
            field_[i][j].y = i;
        }
    }
}

Minesweeper::Minesweeper(size_t width, size_t height, size_t mines_count) : Minesweeper(width, height) {
    non_mines_qty_ = height * width - mines_count;
    SetMinesRandomly(width, height, mines_count);
}

Minesweeper::Minesweeper(size_t width, size_t height, const std::vector<Cell>& cells_with_mines)
    : Minesweeper(width, height) {

    non_mines_qty_ = height * width - cells_with_mines.size();
    SetMinesByList(width, height, cells_with_mines);
}

void Minesweeper::NewGame(size_t width, size_t height, size_t mines_count) {
    ResetField(width, height);
    non_mines_qty_ = height * width - mines_count;
    SetMinesRandomly(width, height, mines_count);
}

void Minesweeper::NewGame(size_t width, size_t height, const std::vector<Cell>& cells_with_mines) {
    ResetField(width, height);
    non_mines_qty_ = height * width - cells_with_mines.size();
    SetMinesByList(width, height, cells_with_mines);
}

void Minesweeper::SetMinesRandomly(size_t width, size_t height, size_t mines_count) {
    std::vector<size_t> cell_id_list(width * height);
    for (size_t i = 0; i < cell_id_list.size(); ++i) {
        cell_id_list[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(cell_id_list.begin(), cell_id_list.end(), g);

    for (size_t i = 0; i < mines_count; ++i) {
        size_t x = cell_id_list[i] % width;
        size_t y = floor(static_cast<double>(cell_id_list[i] / width));
        field_[y][x].is_mine = true;
        UpdateMinesQtyNearbyForNeighbours(field_[y][x]);
    }
}

void Minesweeper::SetMinesByList(size_t width, size_t height, const std::vector<Cell>& cells_with_mines) {

    for (auto& mine : cells_with_mines) {
        field_[mine.y][mine.x].is_mine = true;
        UpdateMinesQtyNearbyForNeighbours(field_[mine.y][mine.x]);
    }
}

void Minesweeper::ResetField(size_t width, size_t height) {
    field_width_ = width, field_height_ = height, field_.clear();
    field_ = Field(height, std::vector<CellInfo>(width));
    game_status_ = GameStatus::NOT_STARTED;
    game_start_time_ = 0;
    game_end_time_ = 0;
    opened_cells_qty_ = 0;
}

void Minesweeper::OpenCell(const Minesweeper::Cell& cell) {

    if (game_status_ == GameStatus::NOT_STARTED) {
        StartGame();
    }

    if (game_status_ == GameStatus::IN_PROGRESS) {
        CellInfo& cell_info = field_[cell.y][cell.x];

        if (cell_info.is_mine) {
            std::time(&game_end_time_);
            game_status_ = GameStatus::DEFEAT;

            for (auto& row : field_) {
                for (auto& el : row) {
                    el.is_opened = true;
                    ++opened_cells_qty_;
                }
            }

        } else if (!cell_info.is_opened && !cell_info.flag) {
            Bfs(cell_info);

            if (opened_cells_qty_ == non_mines_qty_) {
                std::time(&game_end_time_);
                game_status_ = GameStatus::VICTORY;
            }
        }
    }
}

void Minesweeper::MarkCell(const Minesweeper::Cell& cell) {
    if (game_status_ == GameStatus::NOT_STARTED) {
        StartGame();
    }

    if (game_status_ == GameStatus::IN_PROGRESS) {
        field_[cell.y][cell.x].flag = !field_[cell.y][cell.x].flag;
    }
}

Minesweeper::GameStatus Minesweeper::GetGameStatus() const {
    return game_status_;
}

time_t Minesweeper::GetGameTime() const {
    if (game_status_ == GameStatus::IN_PROGRESS) {
        time_t now = std::time(nullptr);
        return lround(difftime(now, game_start_time_));
    }

    if (game_status_ == GameStatus::NOT_STARTED) {
        return 0;
    }

    return lround(difftime(game_end_time_, game_start_time_));
}

Minesweeper::RenderedField Minesweeper::RenderField() const {
    RenderedField rendered_field;
    rendered_field.reserve(field_height_);

    for (auto& i : field_) {
        std::string row;

        for (auto& cell : i) {

            if (cell.is_opened) {

                if (cell.is_mine) {
                    row.append("*");
                    continue;
                }

                if (cell.mines_qty_nearby > 0) {
                    row.append(std::to_string(cell.mines_qty_nearby));
                    continue;
                }

                row.append(".");
                continue;
            }

            if (cell.flag) {
                row.append("?");
                continue;
            }
            row.append("-");
        }

        rendered_field.emplace_back(row);
    }
    return rendered_field;
}

void Minesweeper::StartGame() {
    std::time(&game_start_time_);  // game_start_time_ = std::time(nullptr);
    game_status_ = GameStatus::IN_PROGRESS;
}

void Minesweeper::Bfs(Minesweeper::CellInfo& cell) {
    std::queue<CellInfo*> queue;
    queue.push(&field_[cell.y][cell.x]);

    while (!queue.empty()) {
        CellInfo* current = queue.front();

        if (current->is_opened) {
            queue.pop();
            continue;
        }

        if (!current->is_opened) {
            current->is_opened = true;
            ++opened_cells_qty_;
            if (current->mines_qty_nearby > 0) {
                queue.pop();
                continue;
            }
        }

        std::vector<CellInfo*> neighbours = GetNeighbours(*current);

        for (auto neighbour : neighbours) {

            if (neighbour->is_opened or neighbour->flag) {
                continue;
            }

            queue.push(neighbour);
        }

        queue.pop();
    }
}

std::vector<Minesweeper::CellInfo*> Minesweeper::GetNeighbours(const Minesweeper::Cell& cell) {
    std::vector<CellInfo*> neighbours;
    size_t i = cell.y;
    size_t j = cell.x;

    if (i > 0 && j > 0) {
        neighbours.push_back(&field_[i - 1][j - 1]);
    }

    if (i > 0) {
        neighbours.push_back(&field_[i - 1][j]);
    }

    if (i > 0 && j + 1 < field_width_) {
        neighbours.push_back(&field_[i - 1][j + 1]);
    }

    if (j + 1 < field_width_) {
        neighbours.push_back(&field_[i][j + 1]);
    }

    if (i + 1 < field_height_ && j + 1 < field_width_) {
        neighbours.push_back(&field_[i + 1][j + 1]);
    }

    if (i + 1 < field_height_) {
        neighbours.push_back(&field_[i + 1][j]);
    }

    if (i + 1 < field_height_ && j > 0) {
        neighbours.push_back(&field_[i + 1][j - 1]);
    }

    if (j > 0) {
        neighbours.push_back(&field_[i][j - 1]);
    }

    return neighbours;
}

void Minesweeper::UpdateMinesQtyNearbyForNeighbours(const Minesweeper::CellInfo& cell) {
    std::vector<CellInfo*> neighbours = GetNeighbours(cell);

    for (auto neighbour : neighbours) {
        ++neighbour->mines_qty_nearby;
    }
}