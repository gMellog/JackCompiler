#include "CompilationEngine.h"
#include <cstdlib>
#include <iostream>

//TODO
//MAKE SPECIAL STAGE WHICH WILL CHECK ONLY SYNTAX OF JACK
//AND THEN WHEN WE CHECKED SYNTAX AND IT'S GOOD WE CAN RUN COMPILER
//i hate those fucking checks everywhere

CompilationEngine::CompilationEngine(const std::string& writeFileName, JackTokenizer& pJackTokenizer)
	:
	jackTokenizer{pJackTokenizer},
	symbolTable{},
	newVarEntity{},
	vmWriter{ writeFileName },
	_class{},
	subroutineID{},
	subroutineType{},
	passArgsNum{},
	whileRunningLabelIndex{},
	ifRunningLabelIndex{},
	emptyExpression{}
{
	initCommands();
}

void CompilationEngine::compile()
{
	compileClass();
}

void CompilationEngine::initCommands()
{
	commands.insert({'+', ECOMMAND::ADD});
	commands.insert({ '-', ECOMMAND::SUB });
	commands.insert({ '&', ECOMMAND::AND });
	commands.insert({ '|', ECOMMAND::OR });
	commands.insert({ '<', ECOMMAND::LT });
	commands.insert({ '>', ECOMMAND::GT });
	commands.insert({ '=', ECOMMAND::EQ });
}

void CompilationEngine::initStaticVars()
{
	const auto staticCount = symbolTable.getVarCount(EKIND::STATIC);

	for (int i = 0; i < staticCount; ++i)
	{
		vmWriter.writePush(ESEGMENT::CONST, 0);
		vmWriter.writePop(ESEGMENT::STATIC, i);
	}
}

void CompilationEngine::discardValue(const std::string& str)
{
	fatalExitIfTokenizerEmpty();

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (str == tokenValue)
	{
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat " << str << " cause it's not token value\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatVarLine()
{
	newVarEntity = VarEntity();

	eatClassVarScope();
	eatVarType();
	eatVarId();
	
	symbolTable.define(newVarEntity);

	eatOptionalVarNames();
}

void CompilationEngine::eatClassVarScope()
{
	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	newVarEntity.kind = symbolTable.getKindFromStr(tokenValue);

	++jackTokenizer;
}

void CompilationEngine::eatVarType()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (isType(tokenValue, jackTokenizer.getCurrTokenType()))
	{
		newVarEntity.type = tokenValue;
		++jackTokenizer;
	}
	else
	{
		std::cerr << "I have to eat type, but instead got " << tokenValue << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatVarId()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (jackTokenizer.getCurrTokenType() == JackTokenizer::ETOKEN::IDENTIFIER)
	{
		newVarEntity.name = tokenValue;
		++jackTokenizer;
	}
	else
	{
		std::cerr << "I have to eat id, but instead got something another: " << tokenValue << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatOptionalVarNames()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;
	while (tokenValue == ",")
	{
		++jackTokenizer;
		eatVarId();
		symbolTable.define(newVarEntity);

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}

	discardValue(";");
}

void CompilationEngine::eatFuncRetType()
{
	fatalExitIfTokenizerEmpty();

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;
	const auto token = jackTokenizer.getCurrTokenType();
	
	if (tokenValue == "void" || isType(tokenValue, token))
	{
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat func ret type " << tokenValue << "\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::eatClassID()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (jackTokenizer.getCurrTokenType() == JackTokenizer::ETOKEN::IDENTIFIER)
	{
		_class = std::move(tokenValue);
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat " << tokenValue << " cause it's no identifier\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

}

void CompilationEngine::eatSubroutineType()
{
	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	subroutineType = std::move(tokenValue);
	++jackTokenizer;
}

void CompilationEngine::compileClass()
{
	discardValue("class");
	eatClassID();
	discardValue("{");
	compileClassVarDec();
	compileSubroutine();
	discardValue("}");
}

void CompilationEngine::compileClassVarDec()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	while (tokenValue == "static" || tokenValue == "field")
	{
		eatVarLine();

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
	
	initStaticVars();
}

void CompilationEngine::eatOneOfThem(std::initializer_list<std::string> list)
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	for (const auto& str : list)
	{
		if (tokenValue == str)
		{
			++jackTokenizer;
			return;
		}
	}

	std::cerr << "Can't eat one of ";
	for (const auto& str : list)
	{
		std::cerr << str << " ";
	}
	std::cerr << "\n";

	std::cerr << "Terminating...\n";
	std::exit(1);
}

void CompilationEngine::eatSubroutineID()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (jackTokenizer.getCurrTokenType() == JackTokenizer::ETOKEN::IDENTIFIER)
	{
		subroutineID = std::move(tokenValue);
		++jackTokenizer;
	}
	else
	{
		std::cerr << "Can't eat " << tokenValue << " cause it's no identifier\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::compileSubroutine()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	while (tokenValue == "constructor"||
		   tokenValue == "function"   ||
		   tokenValue == "method"
		)
	{
		symbolTable.startSubroutine();

		eatSubroutineType();
		eatFuncRetType();
		eatSubroutineID();
		discardValue("(");

		compileParameterList();

		discardValue(")");
		compileSubroutineBody();

		fatalExitIfTokenizerEmpty();
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}
}

void CompilationEngine::compileParameterList()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (subroutineType == "method")
	{
		VarEntity thisVarEntity;
		thisVarEntity.name = "this";
		thisVarEntity.kind = EKIND::ARG;
		thisVarEntity.type = _class;

		symbolTable.define(thisVarEntity);
	}

	while (tokenValue != ")")
	{
		if(tokenValue == ",")
			discardValue(",");

		newVarEntity = VarEntity();
		newVarEntity.kind = EKIND::ARG;

		eatVarType();
		eatVarId();

		symbolTable.define(newVarEntity);

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}

		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}

}

void CompilationEngine::compileVarDec()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;
	
	while (tokenValue == "var")
	{
		eatVarLine();

		if (!jackTokenizer.isThereAnyToken())
		{
			tokenizerEmptyOut();
		}
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
	}

	if(tokenValue == ";")
		discardValue(";");
}

void CompilationEngine::compileSubroutineBody()
{
	discardValue("{");

	compileVarDec();

	const std::string funcName = _class + "." + subroutineID;
	const std::size_t subroutineLocals = symbolTable.getVarCount(EKIND::VAR);
	vmWriter.writeFunction(funcName, subroutineLocals);

	if (subroutineType == "constructor")
	{
		const auto staticVarsNum = symbolTable.getVarCount(EKIND::STATIC);
		const auto fieldVarsNum = symbolTable.getVarCount(EKIND::FIELD);
		const auto size = staticVarsNum + fieldVarsNum;

		vmWriter.writePush(ESEGMENT::CONST, size);
		vmWriter.writeCall("Memory.alloc", 1);
		vmWriter.writePop(ESEGMENT::POINTER, 0);
	}

	compileStatements();
	discardValue("}");
}

void CompilationEngine::compileStatements()
{
	while (isThereAnyStatement())
		compileStatement();
}

bool CompilationEngine::isThereAnyStatement() const
{
	bool r = false;

	if (jackTokenizer.isThereAnyToken())
	{
		std::string tokenValue;
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		r = isThatStatement(tokenValue);
	}

	return r;
}

bool CompilationEngine::isThatStatement(const std::string& str) const noexcept
{
	return str == "if" || str == "do" || str == "while" || str == "return" || str == "let";
}

void CompilationEngine::compileStatement()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	if (tokenValue == "if")
	{
		compileIf();
	}
	else if (tokenValue == "do")
	{
		compileDo();
	}
	else if (tokenValue == "while")
	{
		compileWhile();
	}
	else if (tokenValue == "let")
	{
		compileLet();
	}
	else if (tokenValue == "return")
	{
		compileReturn();
	}
	else
	{
		std::cerr << "Unrecognized statement: " << tokenValue << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::subroutineCall()
{
	fatalExitIfTokenizerEmpty();

	std::string subroutineIDCall;
	std::tie(subroutineIDCall, std::ignore) = *jackTokenizer;

	if (jackTokenizer.getCurrTokenType() != JackTokenizer::ETOKEN::IDENTIFIER)
	{
		std::cerr << "Instead identifier for subroutine got " << subroutineIDCall << '\n';
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

	++jackTokenizer;

	std::string nextToken;
	std::tie(nextToken, std::ignore) = *jackTokenizer;

	if (nextToken == "(")
	{
		//so this is method inside our class

		if (subroutineType == "constructor")
		{
			vmWriter.writePush(ESEGMENT::POINTER, 0);
		}
		else
		{
			vmWriter.writePush(ESEGMENT::ARG, 0);
		}

		compileExpressionList();

		const std::string funcName = _class + "." + subroutineIDCall;
		vmWriter.writeCall(funcName, passArgsNum + 1);

		vmWriter.writePop(ESEGMENT::TEMP, 0);
	}
	else if (nextToken == ".")
	{
		//subroutineIDCall could there be an object or its another class
		discardValue(".");

		fatalExitIfTokenizerEmpty();

		std::string funcID;
		std::tie(funcID, std::ignore) = *jackTokenizer;

		const auto [var, index] = symbolTable.getVar(subroutineIDCall);
		++jackTokenizer;

		if (var.isValid())
		{
			if (subroutineType == "method")
			{
				vmWriter.writePush(ESEGMENT::ARG, 0);
				vmWriter.writePop(ESEGMENT::POINTER, 0);
			}
			
			const auto segment = getVarSegment(var);

			vmWriter.writePush(segment, index);

			compileExpressionList();
			const std::string funcName = var.type + "." + funcID;
			vmWriter.writeCall(funcName, passArgsNum + 1);
		}
		else
		{
			//Ok, then subroutineIDCall is probably class name, wdk for sure, so we have to trust until link time
			
			compileExpressionList();
			const std::string funcName = subroutineIDCall + "." + funcID;
			vmWriter.writeCall(funcName, passArgsNum);
		}

		//omit return value
		vmWriter.writePop(ESEGMENT::TEMP, 0);
	}
}

void CompilationEngine::compileExpression(bool letExpr)
{
	compileTerm();
	compileOptionalOpTerm();

	if (letExpr)
	{
		vmWriter.writePop(ESEGMENT::TEMP, 0);
	}
}

void CompilationEngine::compileTerm()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;
	const auto token = jackTokenizer.getCurrTokenType();

	if (token == JackTokenizer::ETOKEN::IDENTIFIER)
	{
		fatalExitIfTokenizerEmpty();

		auto entityID = std::move(tokenValue);
		const auto [var, index] = symbolTable.getVar(entityID);

		++jackTokenizer;

		fatalExitIfTokenizerEmpty();

		std::tie(tokenValue, std::ignore) = *jackTokenizer;

		if (tokenValue == "[")
		{
			if (var.isValid())
			{
				discardValue("[");

				const auto segment = getVarSegment(var);

				if (segment == ESEGMENT::THIS)
				{
					if (subroutineType != "constructor")
					{
						vmWriter.writePush(ESEGMENT::ARG, 0);
						vmWriter.writePop(ESEGMENT::POINTER, 0);
					}
				}

				vmWriter.writePush(segment, index);

				compileExpression();

				discardValue("]");

				vmWriter.writeArithmetic(ECOMMAND::ADD);
				vmWriter.writePop(ESEGMENT::POINTER, 1);
				vmWriter.writePush(ESEGMENT::THAT, 0);
			}
			else
			{
				std::cerr << "Variable is invalid but attempted to use with subscript operator: " << entityID << "\n";
				std::cerr << "Terminating...\n";
				std::exit(1);
			}
		}
		else if (tokenValue == "(")
		{

			if (subroutineType == "constructor")
			{
				vmWriter.writePush(ESEGMENT::POINTER, 0);
			}
			else
			{
				vmWriter.writePush(ESEGMENT::ARG, 0);
			}

			compileExpressionList();

			vmWriter.writeCall(_class + "." + entityID, passArgsNum + 1);
		}
		else if (tokenValue == ".")
		{
			discardValue(".");

			fatalExitIfTokenizerEmpty();

			std::string subroutineCallID;
			std::tie(subroutineCallID, std::ignore) = *jackTokenizer;
			++jackTokenizer;

			if (var.isValid())
			{
				const auto segment = getVarSegment(var);

				if (segment == ESEGMENT::THIS)
				{
					if (subroutineType != "constructor")
					{
						vmWriter.writePush(ESEGMENT::ARG, 0);
						vmWriter.writePop(ESEGMENT::POINTER, 0);
					}
				}

				vmWriter.writePush(segment, index);
			}

			compileExpressionList();

			if (var.isValid())
			{
				vmWriter.writeCall(var.type + "." + subroutineCallID, passArgsNum + 1);
			}
			else
			{
				vmWriter.writeCall(entityID + "." + subroutineCallID, passArgsNum);
			}
		}
		else
		{
			if (var.isValid())
			{
				const auto segment = getVarSegment(var);

				if (segment == ESEGMENT::THIS)
				{
					if (subroutineType != "constructor")
					{
						vmWriter.writePush(ESEGMENT::ARG, 0);
						vmWriter.writePop(ESEGMENT::POINTER, 0);
					}
				}

				vmWriter.writePush(segment, index);
			}
			else
			{
				std::cerr << "Variable " << entityID << " is invalid but we are trying to use it in expression\n";
				std::cerr << "Terminating...\n";
				std::exit(1);
			}
		}
	}
	else if (token == JackTokenizer::ETOKEN::INTEGER_CONSTANT || token == JackTokenizer::ETOKEN::STRING_CONSTANT ||
		tokenValue == "true" || tokenValue == "false" || tokenValue == "null" || tokenValue == "this")
	{
		if (token == JackTokenizer::ETOKEN::INTEGER_CONSTANT)
		{
			vmWriter.writePush(ESEGMENT::CONST, std::stoi(tokenValue));
		}
		else if (token == JackTokenizer::ETOKEN::STRING_CONSTANT)
		{
			vmWriter.writePush(ESEGMENT::CONST, tokenValue.size());
			vmWriter.writeCall("String.new", 1);

			//after String constructor we already have (his this) on top of the stack

			for (const auto ch : tokenValue)
			{
				//for first iteration we have this on top
				vmWriter.writePush(ESEGMENT::CONST, static_cast<int>(ch));
				vmWriter.writeCall("String.appendChar", 2);
				//this method by api return this, so at the begging of the next iteration we will have this on top too
			}

			//still this from String on top of the stack
		}
		else if (tokenValue == "false" || tokenValue == "null")
		{
			vmWriter.writePush(ESEGMENT::CONST, 0);
		}
		else if (tokenValue == "true")
		{
			vmWriter.writePush(ESEGMENT::CONST, 1);
			vmWriter.writeArithmetic(ECOMMAND::NEG);
		}
		else if (tokenValue == "this")
		{
			if (subroutineType == "constructor")
			{
				vmWriter.writePush(ESEGMENT::POINTER, 0);
			}
			else
			{
				vmWriter.writePush(ESEGMENT::ARG, 0);
			}
		}
		else
		{
			std::cerr << "Unrecognized value: " << tokenValue << '\n';
			std::cerr << "Terminating...\n";
			std::exit(1);
		}

		++jackTokenizer;
	}
	else if (tokenValue == "(")
	{
		discardValue("(");

		fatalExitIfTokenizerEmpty();

		compileExpression();

		discardValue(")");
	}
	else if (tokenValue == "-" || tokenValue == "~")
	{
		++jackTokenizer;
		compileTerm();

		if (tokenValue == "-")
		{
			vmWriter.writeArithmetic(ECOMMAND::NEG);
		}
		else
		{
			vmWriter.writeArithmetic(ECOMMAND::NOT);
		}
	}
	else
	{
		std::cerr << "Not valid token: " << tokenValue << " for term\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}
}

void CompilationEngine::compileOptionalOpTerm()
{
	fatalExitIfTokenizerEmpty();
	std::string opToken;
	std::tie(opToken, std::ignore) = *jackTokenizer;

	if (opToken.size() == 0)
	{
		std::cerr << "Wow, we got empty token while operation checking\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

	char ch = opToken[0];

	while(isOp(ch))
	{
		discardValue(opToken);
		compileTerm();

		const auto command = getOpCommand(ch);

		if (command == ECOMMAND::NONE)
		{
			if (ch == '*')
			{
				vmWriter.writeCall("Math.multiply", 2);
			}
			else if (ch == '/')
			{
				vmWriter.writeCall("Math.divide", 2);
			}
			else
			{
				std::cerr << "Undefined command " << ch << "\n";
				std::cerr << "Terminating...\n";
				std::exit(1);
			}
		}
		else
		{
			vmWriter.writeArithmetic(command);
		}

		fatalExitIfTokenizerEmpty();
		
		std::tie(opToken, std::ignore) = *jackTokenizer;

		if (opToken.size() == 0)
		{
			std::cerr << "Wow, we got empty token while operation checking\n";
			std::cerr << "Terminating...\n";
			std::exit(1);
		}

		ch = opToken[0];
	}
}


void CompilationEngine::compileDo()
{
	discardValue("do");
	subroutineCall();
	discardValue(";");
}

void CompilationEngine::compileLet()
{
	discardValue("let");
	fatalExitIfTokenizerEmpty();

	std::string letID;
	std::tie(letID, std::ignore) = *jackTokenizer;

	const auto [assignToVar, index] = symbolTable.getVar(letID);

	if (!assignToVar.isValid())
	{
		std::cerr << "Attempt to assign new value to " << letID << " but it isn't defined\n";
		std::cerr << "Terminating...\n";
		std::exit(1);
	}

	fatalExitIfTokenizerEmpty();

	++jackTokenizer;

	std::string nextToken;
	std::tie(nextToken, std::ignore) = *jackTokenizer;

	const auto segment = getVarSegment(assignToVar);

	if (segment == ESEGMENT::THIS)
	{
		if (subroutineType != "constructor")
		{
			vmWriter.writePush(ESEGMENT::ARG, 0);
			vmWriter.writePop(ESEGMENT::POINTER, 0);
		}
	}

	if (nextToken == "[")
	{
		discardValue(nextToken);

		vmWriter.writePush(segment, index);

		compileExpression();
		vmWriter.writeArithmetic(ECOMMAND::ADD);

		discardValue("]");
		
		discardValue("=");

		const bool itsLet = true;
		compileExpression(itsLet); // Save result in TEMP 0

		vmWriter.writePop(ESEGMENT::POINTER, 1);
		vmWriter.writePush(ESEGMENT::TEMP, 0);
		vmWriter.writePop(ESEGMENT::THAT, 0);
	}
	else
	{
		discardValue("=");
		
		
		compileExpression();

		vmWriter.writePop(segment, index);
	}

	discardValue(";");

}

void CompilationEngine::compileWhile()
{
	discardValue("while");

	const std::string labelStart = "whileStart$" + std::to_string(whileRunningLabelIndex);
	const std::string labelEnd = "whileEnd$" + std::to_string(whileRunningLabelIndex);
	++whileRunningLabelIndex;

	vmWriter.writeLabel(labelStart);
	compileExpression(); // will eat ( ) parantheses
	vmWriter.writeArithmetic(ECOMMAND::NOT);
	vmWriter.writeIf(labelEnd);
	
	discardValue("{");
	
	compileStatements();

	vmWriter.writeGoto(labelStart);
	
	vmWriter.writeLabel(labelEnd);
	
	discardValue("}");
}

void CompilationEngine::compileReturn()
{
	discardValue("return");
	
	fatalExitIfTokenizerEmpty();
	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	//But what if there is no ';' ?

	if (tokenValue != ";")
	{
		compileExpression();
	}
	else
	{
		vmWriter.writePush(ESEGMENT::CONST, 0);
	}
	
	vmWriter.writeReturn();
	discardValue(";");
}

void CompilationEngine::compileIf()
{
	const std::string labelFalse = "ifFalse$" + std::to_string(ifRunningLabelIndex);
	const std::string labelTrue = "ifTrue$" + std::to_string(ifRunningLabelIndex);

	++ifRunningLabelIndex;

	discardValue("if");
	compileExpression();
	vmWriter.writeArithmetic(ECOMMAND::NOT);
	vmWriter.writeIf(labelFalse);
	discardValue("{");
	compileStatements();
	discardValue("}");

	fatalExitIfTokenizerEmpty();

	const auto [tokenValue, tokenTypeStr] = *jackTokenizer;

	if (tokenValue == "else")
	{
		vmWriter.writeGoto(labelTrue);

		discardValue(tokenValue);
		discardValue("{");
		vmWriter.writeLabel(labelFalse);
		compileStatements();
		discardValue("}");

		vmWriter.writeLabel(labelTrue);
	}
	else
	{
		vmWriter.writeLabel(labelFalse);
	}
}

void CompilationEngine::compileExpressionList()
{
	fatalExitIfTokenizerEmpty();

	std::string tokenValue;
	std::tie(tokenValue, std::ignore) = *jackTokenizer;

	passArgsNum = 0u;

	emptyExpression = false;
	while (tokenValue != ")")
	{
		if (tokenValue == "(")
		{
			discardValue("(");
			std::tie(tokenValue, std::ignore) = *jackTokenizer;
			
			if (tokenValue == ")") // empty parameter list
				break;
		}
		else if
			(tokenValue == ",")
			discardValue(",");


		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		compileExpression();
		
		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		fatalExitIfTokenizerEmpty();

		std::tie(tokenValue, std::ignore) = *jackTokenizer;
		
		if(!emptyExpression)
			++passArgsNum;
	}

	discardValue(")");
}

void CompilationEngine::tokenizerEmptyOut()
{
	std::cerr << "Can't eat, cause tokenizer empty\n";
	std::cerr << "Terminating...\n";
	std::exit(1);
}

bool CompilationEngine::isOp(const char ch)
{	
	return ch == '+' || ch == '-' || ch == '*'
		|| ch == '/' || ch == '&' || ch == '|'
		|| ch == '<' || ch == '>' || ch == '=';
}

bool CompilationEngine::isType(const std::string& tokenValue, const JackTokenizer::ETOKEN tokenType) const noexcept
{
	return tokenValue == "int" || tokenValue == "char"
		|| tokenValue == "boolean" 
		|| tokenType == JackTokenizer::ETOKEN::IDENTIFIER;
}

void CompilationEngine::fatalExitIfTokenizerEmpty()
{
	if (!jackTokenizer.isThereAnyToken())
	{
		tokenizerEmptyOut();
	}
}

ECOMMAND CompilationEngine::getOpCommand(char ch) const noexcept
{
	auto r = ECOMMAND::NONE;

	auto it = commands.find(ch);

	if (it != std::end(commands))
	{
		r = it->second;
	}

	return r;
}

ESEGMENT CompilationEngine::getVarSegment(const VarEntity& varEntity) const noexcept
{
	ESEGMENT r;

	switch (varEntity.kind)
	{
		case EKIND::STATIC:
			r = ESEGMENT::STATIC;
			break;

		case EKIND::FIELD:
			r = ESEGMENT::THIS;
			break;

		case EKIND::VAR:
			r = ESEGMENT::LOCAL;
			break;

		case EKIND::ARG:
			r = ESEGMENT::ARG;
			break;
	}

	return r;
}
