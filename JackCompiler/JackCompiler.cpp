#include "JackCompiler.h"
#include "CompilationEngine.h"
#include "JackTokenizer.h"
#include "utils.h"
#include <filesystem>


void JackCompiler::start(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "usage: JackAnalyzer [***.jack | DIRECTORY(with ***.jack files)\n";
		std::exit(1);
	}

	std::vector<std::string> jackFiles;
	
	if (isJackFile(argv[1]))
	{
		jackFiles.push_back(argv[1]);
	}
	else
	{
		try
		{
			for (const auto& file : std::filesystem::directory_iterator(argv[1]))
			{
				const std::string filePath{ file.path().u8string() };

				if (isJackFile(filePath))
				{
					jackFiles.push_back(filePath);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& ex)
		{
			const std::string exceptionMessage = ex.what();
			std::cerr << exceptionMessage.substr(exceptionMessage.find(' ') + 1) << '\n';
			std::exit(1);
		}
	}

	for (const auto& jackFilePath : jackFiles)
	{
		JackTokenizer jackTokenizer{ jackFilePath };
		CompilationEngine compileEng{ getFileNameWithoutExtension(jackFilePath), jackTokenizer};
		compileEng.compile();
	}
}

bool JackCompiler::isJackFile(const std::string& str) const noexcept
{
	return str.size() > 1 ? str.find('.') != std::string::npos ? split(str,'.')[1] == "jack" : false : false;
}

std::string JackCompiler::getFileNameWithoutExtension(const std::string& str) const
{
	return str.size() > 1 ? str.find('.') != std::string::npos ? split(str, '.')[0] : str : str;
}
