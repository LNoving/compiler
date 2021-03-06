#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include<iostream>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include"llvmsql.h"


using namespace llvm;



//===----------------------------------------------------------------------===//
// scanner_begin
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
	tok_eof = -1,

	// commands
	tok_def = -2,
	tok_extern = -3,

	// primary
	tok_identifier = -4,
	tok_number = -5
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int llvmgettok()
{
	static int LastChar = ' ';

	// Skip any whitespace.
	while (isspace(LastChar))
		LastChar = getchar();

	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())))
			IdentifierStr += LastChar;

		if (IdentifierStr == "def")
			return tok_def;
		if (IdentifierStr == "extern")
			return tok_extern;
		return tok_identifier;
	}

	if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
		std::string NumStr;
		do {
			NumStr += LastChar;
			LastChar = getchar();
		} while (isdigit(LastChar) || LastChar == '.');

		NumVal = strtod(NumStr.c_str(), nullptr);
		return tok_number;
	}

	if (LastChar == '#') {
		// Comment until end of line.
		do
			LastChar = getchar();
		while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF)
			return llvmgettok();
	}

	// Check for end of file.  Don't eat the EOF.
	if (LastChar == EOF)
		return tok_eof;

	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;
}

//===----------------------------------------------------------------------===//
// scanner_end
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// parser_begin
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

namespace {

	/// exprAST - Base class for all expression nodes.
	class exprAST 
	{
	public:
		virtual ~exprAST() = default;

		virtual Value *codegen() = 0;
	};

	/// doubleLiteralAST - Expression class for numeric literals like "1.0".
	class doubleLiteralAST : public exprAST 
	{
		double Val;

	public:
		doubleLiteralAST(double Val) : Val(Val) {}

		Value *codegen() override;
	};

	/// VariableExprAST - Expression class for referencing a variable, like "a".
	class VariableExprAST : public exprAST 
	{
		std::string Name;

	public:
		VariableExprAST(const std::string &Name) : Name(Name) {}

		Value *codegen() override;
	};

	/// BinaryExprAST - Expression class for a binary operator.
	class BinaryExprAST : public exprAST 
	{
		char Op;
		std::unique_ptr<exprAST> LHS, RHS;

	public:
		BinaryExprAST(char Op, std::unique_ptr<exprAST> LHS,
			std::unique_ptr<exprAST> RHS)
			: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

		Value *codegen() override;
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public exprAST 
	{
		std::string Callee;
		std::vector<std::unique_ptr<exprAST>> Args;

	public:
		CallExprAST(const std::string &Callee,std::vector<std::unique_ptr<exprAST>> Args)
			: Callee(Callee), Args(std::move(Args)) {}

		Value *codegen() override;
	};

	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class PrototypeAST 
	{
		std::string Name;
		std::vector<std::string> Args;

	public:
		PrototypeAST(const std::string &Name, std::vector<std::string> Args)
			: Name(Name), Args(std::move(Args)) {}

		Function *codegen();
		const std::string &getName() const { return Name; }
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST 
	{
		std::unique_ptr<PrototypeAST> Proto;
		std::unique_ptr<exprAST> Body;

	public:
		FunctionAST(std::unique_ptr<PrototypeAST> Proto,std::unique_ptr<exprAST> Body)
			: Proto(std::move(Proto)), Body(std::move(Body)) {}

		Function *codegen();
	};

} // end anonymous namespace

  /// CurTok/getnextToken - Provide a simple token buffer.  CurTok is the current
  /// token the parser is looking at.  getnextToken reads another token from the
  /// lexer and updates CurTok with its results.
static int CurTok;
static int getnextToken()
{
	return CurTok = llvmgettok();
}

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence()
{
	if (!isascii(CurTok))
		return -1;

	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok];
	if (TokPrec <= 0)
		return -1;
	return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<exprAST> LogError(const char *Str)
{
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
	LogError(Str);
	return nullptr;
}

static std::unique_ptr<exprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<exprAST> ParseNumberExpr()
{
	auto Result = llvm::make_unique<doubleLiteralAST>(NumVal);
	getnextToken(); // consume the number
	return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<exprAST> ParseParenExpr()
{
	getnextToken(); // eat (.
	auto V = ParseExpression();
	if (!V)
		return nullptr;

	if (CurTok != ')')
		return LogError("expected ')'");
	getnextToken(); // eat ).
	return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<exprAST> ParseIdentifierExpr()
{
	std::string IdName = IdentifierStr;

	getnextToken(); // eat identifier.

	if (CurTok != '(') // Simple variable ref.
		return llvm::make_unique<VariableExprAST>(IdName);

	// Call.
	getnextToken(); // eat (
	std::vector<std::unique_ptr<exprAST>> Args;
	if (CurTok != ')')
	{
		while (true)
		{
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;

			if (CurTok != ',')
				return LogError("Expected ')' or ',' in argument list");
			getnextToken();
		}
	}

	// Eat the ')'.
	getnextToken();

	return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<exprAST> ParsePrimary()
{
	switch (CurTok)
	{
	default:
		return LogError("unknown token when expecting an expression");
	case tok_identifier:
		return ParseIdentifierExpr();
	case tok_number:
		return ParseNumberExpr();
	case '(':
		return ParseParenExpr();
	}
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<exprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<exprAST> LHS)
{
	// If this is a binop, find its precedence.
	while (true)
	{
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec)
			return LHS;

		// Okay, we know this is a binop.
		int BinOp = CurTok;
		getnextToken(); // eat binop

						// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary();
		if (!RHS)
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec)
		{
			RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS =
			llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
	}
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<exprAST> ParseExpression()
{
	auto LHS = ParsePrimary();
	if (!LHS)
		return nullptr;

	return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype()
{
	if (CurTok != tok_identifier)
		return LogErrorP("Expected function name in prototype");

	std::string FnName = IdentifierStr;
	getnextToken();

	if (CurTok != '(')
		return LogErrorP("Expected '(' in prototype");

	std::vector<std::string> ArgNames;
	while (getnextToken() == tok_identifier)
		ArgNames.push_back(IdentifierStr);
	if (CurTok != ')')
		return LogErrorP("Expected ')' in prototype");

	// success.
	getnextToken(); // eat ')'.

	return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition()
{
	getnextToken(); // eat def.
	auto Proto = ParsePrototype();
	if (!Proto)
		return nullptr;

	if (auto E = ParseExpression())
		return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	return nullptr;
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr()
{
	if (auto E = ParseExpression())
	{
		// Make an anonymous proto.
		auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
			std::vector<std::string>());
		return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	}
	return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern()
{
	getnextToken(); // eat extern.
	return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// parser_end
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// codegen_begin
//===----------------------------------------------------------------------===//

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> NamedValues;

Value *LogErrorV(const char *Str)
{
	LogError(Str);
	return nullptr;
}

Value* SetAST::codegen()
{
	std::string Name = *(id->id.get());
	Value *V = NamedValues[Name];
	if (!V)
		return LogErrorV("Unknown variable name");
	return V;
}

Value *doubleLiteralAST::codegen()
{
	return ConstantFP::get(TheContext, APFloat(Val));
}

Value *VariableExprAST::codegen()
{
	// Look this variable up in the function.
	Value *V = NamedValues[Name];
	if (!V)
		return LogErrorV("Unknown variable name");
	return V;
}

Value *BinaryExprAST::codegen()
{
	Value *L = LHS->codegen();
	Value *R = RHS->codegen();
	if (!L || !R)
		return nullptr;

	switch (Op) {
	case '+':
		return Builder.CreateFAdd(L, R, "addtmp");
	case '-':
		return Builder.CreateFSub(L, R, "subtmp");
	case '*':
		return Builder.CreateFMul(L, R, "multmp");
	case '<':
		L = Builder.CreateFCmpULT(L, R, "cmptmp");
		// Convert bool 0/1 to double 0.0 or 1.0
		return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
	default:
		return LogErrorV("invalid binary operator");
	}
}

Value *CallExprAST::codegen()
{
	// Look up the name in the global module table.
	Function *CalleeF = TheModule->getFunction(Callee);
	if (!CalleeF)
		return LogErrorV("Unknown function referenced");

	// If argument mismatch error.
	if (CalleeF->arg_size() != Args.size())
		return LogErrorV("Incorrect # arguments passed");

	std::vector<Value *> ArgsV;
	for (unsigned i = 0, e = Args.size(); i != e; ++i)
	{
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back())
			return nullptr;
	}

	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Function *PrototypeAST::codegen()
{
	// Make the function type:  double(double,double) etc.
	std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(TheContext));
	FunctionType *FT =
		FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);

	Function *F =
		Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto &Arg : F->args())
		Arg.setName(Args[Idx++]);

	return F;
}

Function *FunctionAST::codegen()
{
	// First, check for an existing function from a previous 'extern' declaration.
	Function *TheFunction = TheModule->getFunction(Proto->getName());

	if (!TheFunction)
		TheFunction = Proto->codegen();

	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
	Builder.SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	NamedValues.clear();
	for (auto &Arg : TheFunction->args())
		NamedValues[Arg.getName()] = &Arg;

	if (Value *RetVal = Body->codegen())
	{
		// Finish off the function.
		Builder.CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleDefinition()
{
	if (auto FnAST = ParseDefinition())
	{
		if (auto *FnIR = FnAST->codegen())
		{
			fprintf(stderr, "Read function definition:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}
	}
	else
	{
		// Skip token for error recovery.
		getnextToken();
	}
}

static void HandleExtern()
{
	if (auto ProtoAST = ParseExtern())
	{
		if (auto *FnIR = ProtoAST->codegen())
		{
			fprintf(stderr, "Read extern: ");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}
	}
	else
	{
		// Skip token for error recovery.
		getnextToken();
	}
}

static void HandleTopLevelExpression()
{
	// Evaluate a top-level expression into an anonymous function.
	if (auto FnAST = ParseTopLevelExpr())
	{
		if (auto *FnIR = FnAST->codegen())
		{
			fprintf(stderr, "Read top-level expression:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}
	}
	else
	{
		// Skip token for error recovery.
		getnextToken();
	}
}

/// top ::= definition | external | expression | ';'
static void MainLoop()
{
	while (true)
	{
		fprintf(stderr, "YSQL> ");
		switch (CurTok)
		{
		case tok_eof:
			return;
		case ';': // ignore top-level semicolons.
			getnextToken();
			break;
		case tok_def:
			HandleDefinition();
			break;
		case tok_extern:
			HandleExtern();
			break;
		default:
			HandleTopLevelExpression();
			break;
		}
	}
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main()
{
	init_scanner();
	std::unique_ptr<ExprAST> x;
	init_parser();
	try
	{
		auto z = ParseSelectAST();
	}
	catch (std::runtime_error& s)
	{
		std::cout << s.what() << std::endl;
	}
	try
	{
		auto y = ParseCreateTableSimpleAST();
	}
	catch (std::runtime_error& s)
	{
		std::cout << s.what() << std::endl;
	}
 	
	
	
	ParseLiteralAST();
	// Install standard binary operators.
	// 1 is lowest precedence.
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.
	
							   // Prime the first token.
	fprintf(stderr, "YSQL> ");
	getnextToken();

	// Make the module, which holds all the code.
	TheModule = llvm::make_unique<Module>("my cool jit", TheContext);

	// Run the main "interpreter loop" now.
	MainLoop();

	// Print out all of the generated code.
	TheModule->print(errs(), nullptr);

	return 0;
}