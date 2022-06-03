#pragma once

#include <string>
#include <unordered_map>

enum class EKIND
{
	STATIC,
	FIELD,
	ARG,
	VAR,
	NONE
};

struct VarEntity
{
	std::string name;
	std::string type;
	EKIND kind;

	VarEntity()
		:
		name{"null"},
		type{"null"},
		kind{EKIND::NONE}
	{

	}

	bool isValid() const noexcept
	{
		return !(name == "null" && type == "null" && kind == EKIND::NONE);
	}
};

using VarsHashTable = std::unordered_map <std::string, std::pair<VarEntity, int>>;

struct SymbolTable
{
	SymbolTable();

	void startSubroutine();

	void define(const VarEntity& newVar);

	EKIND getKindOf(const std::string& name) const;

	std::size_t getVarCount(const EKIND kind) const;

	std::string getTypeOf(const std::string& name) const;

	std::size_t getIndexOf(const std::string& name) const;

	std::pair<VarEntity, int> getVar(const std::string& name) const;

	EKIND getKindFromStr(const std::string& kindStr) const noexcept;

private:

	void incrementKind(const EKIND kind);
	void clearCounts();

	VarsHashTable classScope;
	VarsHashTable subroutineScope;

	bool isInClassScope;

	std::size_t staticCount;
	std::size_t fieldCount;
	std::size_t argCount;
	std::size_t varCount;

};
