#pragma once

#include <iostream>

struct JackCompiler
{
	void start(int argc, char** argv);

private:
	bool isJackFile(const std::string& str) const noexcept;
	std::string getFileNameWithoutExtension(const std::string& str) const;
};

