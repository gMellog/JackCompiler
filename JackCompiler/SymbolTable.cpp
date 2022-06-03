#include "SymbolTable.h"
#include <iostream>
#include <cstdlib>

SymbolTable::SymbolTable()
	:
	classScope{},
	subroutineScope{},
	isInClassScope{true},
	staticCount{},
	fieldCount{},
	argCount{},
	varCount{}
{
}

void SymbolTable::startSubroutine()
{
	if (isInClassScope)
		isInClassScope = false;

	subroutineScope.clear();
	clearCounts();
}

void SymbolTable::define(const VarEntity& newVar)
{
	if (isInClassScope)
	{
		classScope.insert({ newVar.name, { newVar, getVarCount(newVar.kind) } });
	}
	else
	{
		subroutineScope.insert({ newVar.name, { newVar, getVarCount(newVar.kind) } });
	}

	incrementKind(newVar.kind);
}

std::size_t SymbolTable::getVarCount(const EKIND kind) const
{
	std::size_t r;

	switch (kind)
	{
	case EKIND::ARG:
		r = argCount;
		break;

	case EKIND::FIELD:
		r = fieldCount;
		break;

	case EKIND::STATIC:
		r = staticCount;
		break;

	case EKIND::VAR:
		r = varCount;
		break;

	case EKIND::NONE:
		r = 0;
		break;

	default:
		std::cerr << "Unknown kind\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

	return r;
}

std::pair<VarEntity, int> SymbolTable::getVar(const std::string& name) const
{
	std::pair<VarEntity, int> r;

	auto it = classScope.find(name);

	if (it == std::end(classScope))
	{
		it = subroutineScope.find(name);
		if (it != std::end(subroutineScope))
		{
			r = it->second;
		}
	}
	else
	{
		r = it->second;
	}

	return r;
}

EKIND SymbolTable::getKindFromStr(const std::string& kindStr) const noexcept
{
	auto r = EKIND::NONE;

	if (kindStr == "var")
	{
		r = EKIND::VAR;
	}
	else if (kindStr == "field")
	{
		r = EKIND::FIELD;
	}
	else if (kindStr == "static")
	{
		r = EKIND::STATIC;
	}
	else if (kindStr == "argument")
	{
		r = EKIND::ARG;
	}

	return r;
}

void SymbolTable::incrementKind(const EKIND kind)
{
	switch (kind)
	{
		case EKIND::ARG:
			++argCount;
			break;

		case EKIND::FIELD:
			++fieldCount;
			break;

		case EKIND::STATIC:
			++staticCount;
			break;

		case EKIND::VAR:
			++varCount;
			break;

		case EKIND::NONE:
			break;

		default:
			std::cerr << "Unknown kind\n";
			std::cerr << "Terminating...\n";
			std::exit(1);
	}

}

void SymbolTable::clearCounts()
{
	argCount = 0;
	varCount = 0;
}

EKIND SymbolTable::getKindOf(const std::string& name) const
{
	return getVar(name).first.kind;
}

std::string SymbolTable::getTypeOf(const std::string& name) const
{
	return getVar(name).first.type;
}

std::size_t SymbolTable::getIndexOf(const std::string& name) const
{
	return getVar(name).second;
}
