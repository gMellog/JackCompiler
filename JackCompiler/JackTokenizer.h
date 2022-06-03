#pragma once

#include <string>
#include <fstream>
#include <set>
#include <queue>
#include <functional>

struct JackTokenizer
{
	enum class ETOKEN
	{
		KEYWORD,
		SYMBOL,
		INTEGER_CONSTANT,
		STRING_CONSTANT,
		IDENTIFIER,
		NONE
	};

	explicit JackTokenizer(const std::string& path);

	void operator++();

	bool isThereAnyToken() const noexcept;

	std::pair<std::string, std::string> operator*() const;

	std::string getTokenString(const ETOKEN token) const;
	ETOKEN getCurrTokenType() const noexcept;

private:

	void initSetKeywords();
	void initSetSymbols();
	void initOperationsQueue();

	bool reinitLineAndIndex();
	bool trySkipAllSpaces();
	bool trySkipComments();
	bool tryConsumeSymbol();
	bool tryConsumeStringConstant();
	bool tryConsumeIntegerConstant();
	bool tryConsumeKeywordOrIdentifier();

	bool isIdentifierToken(const std::string& str) const noexcept;

	bool inSetSymbols(char ch) const noexcept;
	bool symbolAlreadySet() const noexcept;

	std::ifstream readFile;
	std::string line;
	std::size_t i_ch;
	std::set<std::string> setKeywords;
	std::set<char> setSymbols;
	mutable std::pair<std::string, ETOKEN> currToken;
	std::vector<std::function<bool()>> operationsQueue;

	bool multiCommentEndNotFound;

};

