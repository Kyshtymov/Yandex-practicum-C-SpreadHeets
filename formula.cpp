#include "formula.h"


#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, const FormulaError& fe) {
    return output << fe.ToString();
}


FormulaError::FormulaError(Category category)
        : category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}


std::string_view FormulaError::ToString() const{
    switch (category_) {
        case FormulaError::Category::Ref: 
            return "#REF!";
        case FormulaError::Category::Value: 
            return "#VALUE!";
        case FormulaError::Category::Arithmetic: 
            return "#ARITHM!";
        default: return "#ERROR!";
    }
}


namespace {
Formula::Formula(std::string expression) try :
        ast_(ParseFormulaAST(expression)){
    }
    catch(...){
        throw FormulaException("FormulaError");
    }

FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const  {
    try {
        return ast_.Execute(sheet);
    } 
    catch (const FormulaError& fe) {
        return fe;
    } catch (...) {
        return FormulaError(FormulaError::Category::Value);
    }
}

std::string Formula::GetExpression() const  {
    std::stringstream str;
    ast_.PrintFormula(str);
    return str.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    auto cells_list = ast_.GetCells();
    std::vector<Position> result(cells_list.begin(), cells_list.end());
    
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    
    return result;
}
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}