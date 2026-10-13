#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}
void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid())
        throw InvalidPositionException("");
    
    std::unique_ptr<Cell> cell_check = std::make_unique<Cell>(*this);
    cell_check->Set(std::move(text));
    
    try {
        std::set<Position> pos_ref;
        SearchCycleDependence(pos_ref, pos, cell_check.get());
        
        if (cells_.count(pos.row) && cells_[pos.row].count(pos.col)) {
            Cell* existing_cell = cells_[pos.row][pos.col].get();
            if (existing_cell) {
                RemoveCellReferences(pos, existing_cell);
            }
        }
        
        cells_[pos.row][pos.col] = std::move(cell_check);
        
        AddCellReferences(pos, cells_[pos.row][pos.col].get());
        
        pos_ref.clear();
        InvalidCashes(pos_ref, pos);
    }
    catch(const CircularDependencyException& cde){
        throw cde;
    }
}

void Sheet::SearchCycleDependence(std::set<Position>& set_pos_ref, 
                                 Position target_pos, 
                                 const CellInterface* current_cell) {
    if (!current_cell) {
        return;
    }
    
    std::vector<Position> referenced_positions = current_cell->GetReferencedCells();
    
    for (const Position& ref_pos : referenced_positions) {
        
        if (ref_pos == target_pos) {
            throw CircularDependencyException("CircularDependenceError");
        }
        
        if (set_pos_ref.insert(ref_pos).second) {
            const CellInterface* ref_cell = GetCell(ref_pos);
            SearchCycleDependence(set_pos_ref, target_pos, ref_cell);
        }
    }
}

void Sheet::InvalidCashes(std::set<Position>& set_cells_pos, Position cell_search_pos){
    

    if (!backward_references_.count(cell_search_pos)) 
        return;
    
    std::set<Position> cell_invalid_pos = backward_references_[cell_search_pos];

    for (auto pos : cell_invalid_pos){
        Cell* cell_vect = static_cast<Cell*>(GetCell(pos));
        if (cell_vect && set_cells_pos.insert(pos).second){
            cell_vect->RecomputeCache();
            InvalidCashes(set_cells_pos, pos);
        }
    }
}

void Sheet::RemoveCellReferences(Position source_pos, Cell* cell){
    std::vector<Position> cells_forward = cell->GetReferencedCells();
    for (auto pos : cells_forward){
        if (backward_references_.count(pos)) {
            backward_references_[pos].erase(source_pos);
            if (backward_references_[pos].empty()) {
                backward_references_.erase(pos);
            }
        }
    }
}

void Sheet::AddCellReferences(Position source_pos, Cell* cell){
    std::vector<Position> cells_forward = cell->GetReferencedCells();
    for (auto pos : cells_forward){
        if (!pos.IsValid())
            continue;
        
        if (!cells_[pos.row][pos.col]) {
            cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
            cells_[pos.row][pos.col]->Set("");
        }
        
        backward_references_[pos].insert(source_pos);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid())
        throw InvalidPositionException("");

    auto row_it = cells_.find(pos.row);
    if (row_it == cells_.end()) {
        return nullptr;
    }
    
    auto col_it = row_it->second.find(pos.col);
    if (col_it == row_it->second.end() || !col_it->second) {
        return nullptr;
    }
    
    return col_it->second.get();
}

CellInterface* Sheet::GetCell(Position pos) {  
    if (!pos.IsValid())
        throw InvalidPositionException("");

    auto row_it = cells_.find(pos.row);
    if (row_it == cells_.end()) {
        return nullptr;
    }
    
    auto col_it = row_it->second.find(pos.col);
    if (col_it == row_it->second.end() || !col_it->second) {
        return nullptr;
    }
    
    return col_it->second.get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("");

    auto row_it = cells_.find(pos.row);
    if (row_it == cells_.end()) {
        return;
    }
    
    auto col_it = row_it->second.find(pos.col);
    if (col_it == row_it->second.end()) {
        return;
    }
    
    
    if (backward_references_.count(pos)) {
        col_it->second->Set("");
        return;
    }
    
    col_it->second.reset();
    row_it->second.erase(col_it);
    
    if (row_it->second.empty()) {
        cells_.erase(row_it);
    }
}

Size Sheet::GetPrintableSize() const {    
    if (cells_.empty()) {
        return {0, 0};
    }
    
    int max_row = cells_.rbegin()->first; 
    
    int max_col = 0;
    for (const auto& [row, cols_map] : cells_) { 
        if (!cols_map.empty()) {
            int max_col_in_row = cols_map.rbegin()->first; 
            if (max_col < max_col_in_row) {
                max_col = max_col_in_row;
            }
        }
    }
    return {max_row + 1, max_col + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
     Size size = GetPrintableSize();
    
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            
            if (cells_.count(row) > 0 && 
                cells_.at(row).count(col) > 0 && 
                cells_.at(row).at(col)) {                
                const auto& value = cells_.at(row).at(col)->GetValue();
                std::visit([&output](const auto& val) {
                    output << val;
                }, value);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            
            if (cells_.count(row) > 0 && 
                cells_.at(row).count(col) > 0 && 
                cells_.at(row).at(col)) {                
                output << cells_.at(row).at(col)->GetText();
            }
        }
        output << '\n';
    }
}


bool Sheet::IsValidPosition(Position pos) const {   
    if (pos.row < 0 || pos.col < 0) 
        return false;
    
    auto row_it = cells_.find(pos.row);
    if (row_it == cells_.end()) {
        return false;
    }
    
    auto col_it = row_it->second.find(pos.col);
    return col_it != row_it->second.end() && col_it->second != nullptr;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}