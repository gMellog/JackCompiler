#pragma once
#include <string>
#include <fstream>
#include <map>

enum class ESEGMENT
{
	CONST,
	LOCAL,
	ARG,
	STATIC,
	THIS,
	THAT,
	POINTER,
	TEMP
};

enum class ECOMMAND
{
	ADD,
	SUB,
	NEG,
	EQ,
	GT,
	LT,
	AND,
	OR,
	NOT,
	NONE
};

struct VMWriter
{
	explicit VMWriter(const std::string& vmFileName);

	void writePush(const ESEGMENT segment, int index);
	void writePop(const ESEGMENT segment, int index);
	
	void writeArithmetic(const ECOMMAND command);
	
	void writeLabel(const std::string& label);
	void writeGoto(const std::string& label);
	void writeIf(const std::string& label);

	void writeCall(const std::string& name, const std::size_t nArgs);
	void writeFunction(const std::string& name, const std::size_t nLocals);
	void writeReturn();

	std::string getSegmentStr(const ESEGMENT segment);
	std::string getCommandStr(const ECOMMAND command);

private:

	void initSegmentMap();
	void initCommandMap();

	std::ofstream of;

	std::map<ESEGMENT, std::string> segmentMap;
	std::map<ECOMMAND, std::string> commandMap;
};

