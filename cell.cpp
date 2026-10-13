#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


void EmptyImpl::Set(std::string) {
    return;
}

void EmptyImpl::Clear() {
    return;
}

CellInterface::Value EmptyImpl::GetValue(const SheetInterface& sheet) const {
    return "";
}

std::string EmptyImpl::GetText() const {
    return "";
}

void TextImpl::Set(std::string text){
    str_ = text;
}

void TextImpl::Clear(){
    str_ = "";
}

CellInterface::Value TextImpl::GetValue(const SheetInterface& sheet) const {
    if (str_.size() == 0 || str_ == std::to_string(ESCAPE_SIGN))
        return "";

    if (str_[0] == ESCAPE_SIGN)
        return str_.substr(1);

    return str_;
}

std::string TextImpl::GetText() const {
    
    return str_;
}

void FormulaImpl::Set(std::string expression) {
    Clear();
    formula_ = ParseFormula(expression);
}

void FormulaImpl::Clear() {
    formula_.reset();
    formula_ = nullptr;
}

CellInterface::Value FormulaImpl::GetValue(const SheetInterface& sheet) const {
    auto result = formula_->Evaluate(sheet);
    
    
    if (std::holds_alternative<double>(result)) {
        return std::get<double>(result);
    } else {
        return std::get<FormulaError>(result);
    }
}

std::string FormulaImpl::GetText() const{
    if (!formula_) {
        return "";
    }
    return "=" + formula_->GetExpression();
}

Cell::Cell(SheetInterface& sheet):impl_(std::make_unique<EmptyImpl>()), sheet_(sheet){}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    Clear();

    cache_valid_ = false;
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else{
        if (text.size() > 1 && text[0] == FORMULA_SIGN){
            auto formula_impl = std::make_unique<FormulaImpl>();
            formula_impl->Set(text.substr(1));
            impl_ = std::move(formula_impl);
        }
        else{
            auto text_impl = std::make_unique<TextImpl>();
            text_impl->Set(text);
            impl_ = std::move(text_impl);
        }
    }

    RecomputeCache();
}

void Cell::Clear() {
    impl_.reset();
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (impl_->IsFormula() && cache_valid_)
        return cache_;
    
    return impl_->GetValue(sheet_);
}

void Cell::RecomputeCache(){
    cache_valid_ = false;
    if (impl_->IsFormula()){
        Cell::Value value = impl_->GetValue(sheet_);
        if (std::holds_alternative<double>(value)) {
            cache_ = std::get<double>(value);
            cache_valid_ = true;
        }
    }
}


std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}


std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}