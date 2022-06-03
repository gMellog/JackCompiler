#include "VMWriter.h"


VMWriter::VMWriter(const std::string& vmFileName)
	:
	of{vmFileName + ".vm", std::ios_base::out | std::ios_base::trunc},
	segmentMap{},
	commandMap{}
{
	initSegmentMap();
	initCommandMap();
}

void VMWriter::writePush(const ESEGMENT segment, int index)
{
	of << "push " << getSegmentStr(segment) << ' ' << index << '\n';
}

void VMWriter::writePop(const ESEGMENT segment, int index)
{
	of << "pop " << getSegmentStr(segment) << ' ' << index << '\n';
}

void VMWriter::writeArithmetic(const ECOMMAND command)
{
	of << getCommandStr(command) << '\n';
}

void VMWriter::writeLabel(const std::string& label)
{
	of << "label " << label << '\n';
}

void VMWriter::writeGoto(const std::string& label)
{
	of << "goto " << label << '\n';
}

void VMWriter::writeIf(const std::string& label)
{
	of << "if-goto " << label << '\n';
}

void VMWriter::writeCall(const std::string& name, const std::size_t nArgs)
{
	of << "call " << name << ' ' << nArgs << '\n';
}

void VMWriter::writeFunction(const std::string& name, const std::size_t nLocals)
{
	of << "function " << name << ' ' << nLocals << '\n';
}

void VMWriter::writeReturn()
{
	of << "return\n";
}

void VMWriter::initSegmentMap()
{
	segmentMap.insert({ ESEGMENT::ARG, "argument"});
	segmentMap.insert({ ESEGMENT::CONST, "constant" });
	segmentMap.insert({ ESEGMENT::LOCAL, "local" });
	segmentMap.insert({ ESEGMENT::POINTER, "pointer" });

	segmentMap.insert({ ESEGMENT::STATIC, "static" });
	segmentMap.insert({ ESEGMENT::TEMP, "temp" });
	segmentMap.insert({ ESEGMENT::THAT, "that" });
	segmentMap.insert({ ESEGMENT::THIS, "this" });
}

void VMWriter::initCommandMap()
{
	commandMap.insert({ECOMMAND::ADD, "add"});
	commandMap.insert({ ECOMMAND::AND, "and" });
	commandMap.insert({ ECOMMAND::EQ, "eq" });
	commandMap.insert({ ECOMMAND::GT, "gt" });

	commandMap.insert({ ECOMMAND::LT, "lt" });
	commandMap.insert({ ECOMMAND::NEG, "neg" });
	commandMap.insert({ ECOMMAND::NOT, "not" });
	commandMap.insert({ ECOMMAND::OR, "or" });
	commandMap.insert({ ECOMMAND::SUB, "sub" });
}

std::string VMWriter::getSegmentStr(const ESEGMENT segment)
{
	std::string r;

	auto it = segmentMap.find(segment);

	if (it != std::end(segmentMap))
	{
		r = it->second;
	}

	return r;
}

std::string VMWriter::getCommandStr(const ECOMMAND command)
{
	std::string r;

	auto it = commandMap.find(command);

	if (it != std::end(commandMap))
	{
		r = it->second;
	}

	return r;
}