#pragma once

#include "common.h"
#include "formula.h"
#include <forward_list>

class Impl{
public:
    virtual ~Impl() = default;
    virtual void Set(std::string text) = 0;
    virtual void Clear() = 0;
    virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual bool IsFormula() = 0;
};


class EmptyImpl : public Impl{
public:
    EmptyImpl() = default;
    void Set(std::string) override;
    void Clear() override;
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const;
    bool IsFormula() override{
        return false;
    };
};

class TextImpl : public Impl{
public:
    TextImpl() = default;
    void Set(std::string text) override;
    void Clear() override;
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const; 
    bool IsFormula() override{
        return false;
    };
private:
    std::string str_;
};

class FormulaImpl : public Impl{
public:
    FormulaImpl() = default;
    void Set(std::string expression) override;
    void Clear() override;
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const;
    bool IsFormula() override{
        return true;
    };
private:
    std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();
    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    void RecomputeCache();
    //возвращает вектор позиций ячеек, на которые ссылается формула текущей ячейки. Нужно для выявления циклической зависимости
    std::vector<Position> GetReferencedCells() const;
private:
    std::unique_ptr<Impl> impl_;
    //ссылка на таблицу для вычисления формул с ссылками
    SheetInterface& sheet_;
    //булево значение необходимое для индикации, что кэш валидный или нет
    //если кэш валидный, то при вычислении формулы будет вызываться не класс Formula, значение cash_
    //если кэш инвалидный, то при следующем вычислении формулы cash_valid_ = true, cash_ = результат от GetValue()
    bool cache_valid_ = false;
    //Кэш, чтобы не вычислять формулу повторно
    double cache_;

};