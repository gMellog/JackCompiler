#include "JackTokenizer.h"
#include <iostream>

JackTokenizer::JackTokenizer(const std::string& path)
	:
	readFile{path},
	line{},
	i_ch{},
	setKeywords{},
	setSymbols{},
	currToken{},
	operationsQueue{},
	multiCommentEndNotFound{}
{
	initSetKeywords();
	initSetSymbols();
	initOperationsQueue();
	++(*this);
}

void JackTokenizer::operator++()
{
	if (!reinitLineAndIndex())
	{
		return;
	}

	if (!trySkipAllSpaces() || !trySkipComments())
	{
		if (!multiCommentEndNotFound)
		{
			++(*this);
		}
		
		return;
	}


	for (const auto& oper : operationsQueue)
	{
		if (oper())
		{
			return;
		}
	}

	std::cerr << "Unrecognized stream of tokens\n";
	std::cerr << "Terminating...\n";
	std::exit(1);
}

bool JackTokenizer::isThereAnyToken() const noexcept
{
	return currToken.second != ETOKEN::NONE;
}

std::pair<std::string, std::string> JackTokenizer::operator*() const
{
	return { currToken.first, getTokenString(currToken.second) };
}

JackTokenizer::ETOKEN JackTokenizer::getCurrTokenType() const noexcept
{
	return currToken.second;
}

void JackTokenizer::initSetKeywords()
{
	setKeywords.insert(
	{   "class", "constructor", "function",
		"method", "field", "static", "var",
		"int", "char", "boolean", "void", "true",
		"false", "null", "this", "let", "do",
		"if", "else", "while", "return"
	});
}

void JackTokenizer::initSetSymbols()
{
	setSymbols.insert(
	{ '{', '}', '(',
		')', '[', ']', '.',
		',', ';', '+', '-', '*',
		'/', '&', '|', '<', '>',
		'=', '~'
	});
}

void JackTokenizer::initOperationsQueue()
{
	operationsQueue.push_back([this]() { return tryConsumeSymbol(); });
	operationsQueue.push_back([this]() { return tryConsumeStringConstant(); });
	operationsQueue.push_back([this]() { return tryConsumeIntegerConstant(); });
	operationsQueue.push_back([this]() { return tryConsumeKeywordOrIdentifier(); });
}

bool JackTokenizer::reinitLineAndIndex()
{
	while (line.size() == 0 || i_ch == line.size())
	{
		if (!std::getline(readFile, line))
		{
			currToken.second = ETOKEN::NONE;
			return false;
		}

		i_ch = 0;
	}

	return true;
}

bool JackTokenizer::trySkipAllSpaces()
{
	while (std::isspace(line[i_ch]))
		++i_ch;

	auto prev_i_ch = i_ch;
	return i_ch != line.size();
}

bool JackTokenizer::trySkipComments()
{
	if (line[i_ch] == '/')
	{
		char prevCh = line[i_ch];
		++i_ch;

		if (i_ch == line.size())
		{
			std::cerr << "/ ends a line, which isn't correct\n";
			std::exit(1);
		}

		if (line[i_ch] == '/')
		{
			while (i_ch != line.size())
				++i_ch;

			return false;
		}
		else if (std::isspace(line[i_ch]))
		{
			--i_ch;
		}
		else if (line[i_ch] == '*')
		{
			while(1)
			{
				i_ch += 1;

				if (i_ch >= line.size())
				{
					if (!reinitLineAndIndex())
					{
						multiCommentEndNotFound = true;
						return false;
					}
				}

				if (line[i_ch] == '*')
				{
					if ((i_ch < line.size()) && ((i_ch + 1) < line.size()) && (line[i_ch + 1] == '/'))
					{
						i_ch += 2;

						return i_ch < line.size();
					}
				}
			}

		}
		else
		{
			std::cerr << "Unknown sequence of symbols: " << prevCh << line[i_ch] << '\n';
			std::cerr << "Terminating...\n" << prevCh << line[i_ch] << '\n';
			std::exit(1);
		}
	}

	return true;
}

bool JackTokenizer::tryConsumeSymbol()
{
	auto it = setSymbols.find(line[i_ch]);

	if (it != std::end(setSymbols))
	{
		currToken.first = line[i_ch];
		currToken.second = ETOKEN::SYMBOL;
		++i_ch;

		return true;
	}

	return false;
}

bool JackTokenizer::tryConsumeStringConstant()
{
	if (line[i_ch] == '\"')
	{
		std::string token;

		++i_ch;
		for (; line[i_ch] != '\"'; ++i_ch)
			token += line[i_ch];

		currToken.first = std::move(token);
		currToken.second = ETOKEN::STRING_CONSTANT;

		++i_ch;
		return true;
	}

	return false;
}

bool JackTokenizer::tryConsumeIntegerConstant()
{
	if (std::isdigit(line[i_ch]))
	{
		std::string token;

		for (; !isspace(line[i_ch]) && !(inSetSymbols(line[i_ch])) && line.size() > i_ch; ++i_ch)
			token += line[i_ch];

		currToken.first = std::move(token);
		currToken.second = ETOKEN::INTEGER_CONSTANT;

		return true;
	}

	return false;
}

bool JackTokenizer::tryConsumeKeywordOrIdentifier()
{
	std::string token;

	for (; !std::isspace(line[i_ch]) && !(inSetSymbols(line[i_ch])) && line.size() > i_ch; ++i_ch)
		token += line[i_ch];

	auto setIt = setKeywords.find(token);

	if (setIt != std::end(setKeywords))
	{
		currToken.first = std::move(token);
		currToken.second = ETOKEN::KEYWORD;
		return true;
	}
	else if(isIdentifierToken(token))
	{
		currToken.first = std::move(token);
		currToken.second = ETOKEN::IDENTIFIER;
		return true;
	}
	else
	{
		currToken.second = ETOKEN::NONE;
		return false;
	}
}

bool JackTokenizer::isIdentifierToken(const std::string& str) const noexcept
{
	bool r = false;
	const char underscore = 95;

	if (str.size() != 0 && !std::isdigit(str[0]))
	{
		for (const char ch : str)
		{
			if (!std::isalpha(ch) && ch != underscore && !std::isdigit(ch))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool JackTokenizer::inSetSymbols(char ch) const noexcept
{
	return setSymbols.find(ch) != std::end(setSymbols);
}

bool JackTokenizer::symbolAlreadySet() const noexcept
{
	return currToken.first.size() ? currToken.first[0] == '&' ? currToken.first.size() != 1 ? true : false : false : false;
}

std::string JackTokenizer::getTokenString(const ETOKEN token) const
{
	std::string r;

	if (token == ETOKEN::INTEGER_CONSTANT)
	{
		r = "integerConstant";
	}
	else if (token == ETOKEN::KEYWORD)
	{
		r = "keyword";
	}
	else if (token == ETOKEN::SYMBOL)
	{
		r = "symbol";
	}
	else if (token == ETOKEN::IDENTIFIER)
	{
		r = "identifier";
	}
	else if (token == ETOKEN::STRING_CONSTANT)
	{
		r = "stringConstant";
	}
	else
	{
		r = "none";
	}

	return r;
}



