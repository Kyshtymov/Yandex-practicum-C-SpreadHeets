#pragma once

#include "common.h"

#include <functional>
#include <set>
#include <map>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();
    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
private:
    std::map<int, std::map<int, std::unique_ptr<Cell>>> cells_;
    //словарь для нахождения обратных зависимостей для инвалидации кэша
    std::map<Position, std::set<Position>> backward_references_;
    void SearchCycleDependence(std::set<Position>& visited_positions, 
                                 Position target_pos, 
                                 const CellInterface* current_cell);
    void InvalidCashes(std::set<Position>& set_cells_pos, Position cell_search_pos);
    void RemoveCellReferences(Position source_pos, Cell* cell);
    void AddCellReferences(Position source_pos, Cell* cell); 
    bool IsValidPosition(Position pos) const;  
};