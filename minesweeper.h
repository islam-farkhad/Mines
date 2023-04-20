#pragma once

#include <string>
#include <vector>
#include <ctime>

class Minesweeper {
public:
    struct Cell {
        size_t x = 0;
        size_t y = 0;
    };

    struct CellInfo : Cell {
        bool flag = false;
        bool is_mine = false;
        bool is_opened = false;
        size_t mines_qty_nearby = 0;
    };

    enum class GameStatus {
        NOT_STARTED,
        IN_PROGRESS,
        VICTORY,
        DEFEAT,
    };

    using RenderedField = std::vector<std::string>;
    using Field = std::vector<std::vector<CellInfo>>;

    Minesweeper(size_t width, size_t height);
    Minesweeper(size_t width, size_t height, size_t mines_count);
    Minesweeper(size_t width, size_t height, const std::vector<Cell>& cells_with_mines);

    void NewGame(size_t width, size_t height, size_t mines_count);
    void NewGame(size_t width, size_t height, const std::vector<Cell>& cells_with_mines);

    void OpenCell(const Cell& cell);
    void MarkCell(const Cell& cell);

    GameStatus GetGameStatus() const;
    time_t GetGameTime() const;

    RenderedField RenderField() const;

private:
    std::vector<CellInfo*> GetNeighbours(const Cell& cell);
    void UpdateMinesQtyNearbyForNeighbours(const CellInfo& cell);
    void Bfs(CellInfo& cell);
    void StartGame();
    void ResetField(size_t width, size_t height);
    void SetMinesRandomly(size_t width, size_t height, size_t mines_count);
    void SetMinesByList(size_t width, size_t height, const std::vector<Cell>& cells_with_mines);

    size_t field_width_;
    size_t field_height_;
    GameStatus game_status_ = GameStatus::NOT_STARTED;
    std::time_t game_start_time_ = 0;
    std::time_t game_end_time_ = 0;
    Field field_;
    size_t opened_cells_qty_ = 0;
    size_t non_mines_qty_;
};
