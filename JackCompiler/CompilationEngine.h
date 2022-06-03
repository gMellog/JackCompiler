#pragma once

#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"
#include <map>

struct CompilationEngine
{
	CompilationEngine(const std::string& writeFileName, JackTokenizer& pJackTokenizer);

	void compile();

private:

	void initCommands();
	void initStaticVars();
	void discardValue(const std::string& str);
	void eatOneOfThem(std::initializer_list<std::string> list);

	void eatSubroutineID();

	void eatVarLine();
	void eatClassVarScope();
	void eatVarType();
	void eatVarId();
	void eatOptionalVarNames();

	void eatFuncRetType();
	void eatClassID();

	void eatSubroutineType();

	void compileClass();
	void compileClassVarDec();
	void compileSubroutine();
	void compileParameterList();
	void compileVarDec();

	void compileSubroutineBody();

	void compileStatements();

	bool isThereAnyStatement() const;
	bool isThatStatement(const std::string& str) const noexcept;
	void compileStatement();
	void subroutineCall();

	void compileOptionalOpTerm();

	void compileDo();
	void compileLet();
	void compileWhile();
	void compileReturn();

	void compileIf();
	void compileExpression(bool letExpr = false);
	void compileTerm();
	void compileExpressionList();

	void tokenizerEmptyOut();

	bool isOp(const char ch);
	bool isType(const std::string& tokenValue, const JackTokenizer::ETOKEN tokenType) const noexcept;

	void fatalExitIfTokenizerEmpty();

	ECOMMAND getOpCommand(char ch) const noexcept;

	ESEGMENT getVarSegment(const VarEntity& varEntity) const noexcept;

	JackTokenizer& jackTokenizer;
	SymbolTable symbolTable;
	VarEntity newVarEntity;

	VMWriter vmWriter;

	std::string _class;
	std::string subroutineID;
	std::string subroutineType;

	std::size_t passArgsNum;

	std::map<char, ECOMMAND> commands;

	std::size_t whileRunningLabelIndex;
	std::size_t ifRunningLabelIndex;

	bool emptyExpression;
};

