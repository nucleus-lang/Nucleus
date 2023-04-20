#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <vector>
#include "CodeGen.hpp"

#define NEW_TYPE(x) struct x : public Type { llvm::Type* codegen() override; }

#define ARGUMENT_LIST() std::vector<std::unique_ptr<AST::Expression>>

struct AST
{
	static llvm::Value* CurrInst;
	static std::string CurrentIdentifier;

	struct Type
	{
		virtual ~Type() = default;

		bool is_unsigned = false;

		virtual llvm::Type* codegen() = 0;
	};

	static AST::Type* current_proto_type;

	struct Expression
	{
		virtual ~Expression() = default;

		bool _getPointer = false;

		void GetPointer();

		bool onlyLoad = false;

		bool is_unsigned = true;

		llvm::Value* codegenOnlyLoad();

		llvm::Value* CurrentInstruction();

		virtual llvm::Value* codegen() = 0;
	};

	NEW_TYPE(i1);
	NEW_TYPE(i8);
	NEW_TYPE(i16);
	NEW_TYPE(i32);
	NEW_TYPE(i64);
	NEW_TYPE(i128);

	static std::unique_ptr<Expression> ExprError(std::string str);

	struct Number : public Expression
	{
		bool isDouble = false, isInt = false, isFloat = false;
		int64_t intValue = 0;
		uint64_t uintValue = 0;
		double doubleValue = 0;
		float floatValue = 0;
		unsigned bit = 32;
		std::string valueAsString;

		Number(std::string val, bool is_uns = false) 
		{
			is_unsigned = is_uns;
			//std::cout << "Number Input: " << val << "\n";
			if (val.find(".") != std::string::npos) {
				isFloat = val.back() == 'f';
				isDouble = !isFloat;

				if (isFloat) floatValue = std::stof(val);
				else doubleValue = std::stod(val);
			} else {
				isInt = true;

				if(!is_unsigned) intValue = std::stoi(val);
				else uintValue = std::stoul(val);
			}

			valueAsString = val;
		}

		int32_t return_i32()
		{
			return intValue;
		}

		llvm::Value* codegen() override;
	};

	struct Call : public Expression
	{
		std::string Callee;
		ARGUMENT_LIST() Args;

		Call(const std::string &Callee,
			  ARGUMENT_LIST() Args)
		: Callee(Callee), Args(std::move(Args)) {}

		llvm::Value* codegen() override;
	};

	struct Variable : public Expression
	{
		std::string Name;

		std::unique_ptr<Type> T;

		Variable(std::unique_ptr<Type> T, const std::string& Name) : T(std::move(T)), Name(Name) {}

		llvm::Value* codegen() override;
	};

	struct Return : public Expression
	{
		std::unique_ptr<Expression> Expr;

		Return(std::unique_ptr<Expression> Expr) : Expr(std::move(Expr)) {}

		llvm::Value* codegen() override;
	};

	struct Alloca : public Expression
	{
		std::unique_ptr<Type> T;
		std::string VarName;

		bool noLoad = false;

		Alloca(std::unique_ptr<Type> T, std::string VarName) : T(std::move(T)), VarName(VarName) {}

		llvm::Value* codegen() override;
	};

	struct Store : public Expression
	{
		std::unique_ptr<Expression> Target;
		std::unique_ptr<Expression> Value;

		Store(std::unique_ptr<Expression> Target, std::unique_ptr<Expression> Value) : Target(std::move(Target)), Value(std::move(Value)) {}

		llvm::Value* codegen() override;
	};

	struct Load : public Expression
	{
		std::string Name;
		std::unique_ptr<Type> T;
		std::unique_ptr<Expression> Target;

		Load(std::string Name, std::unique_ptr<Type> T, std::unique_ptr<Expression> Target) : Name(Name), T(std::move(T)), Target(std::move(Target)) {}

		llvm::Value* codegen() override;
	};

	struct Add : public Expression
	{
		std::unique_ptr<Expression> Target;
		std::unique_ptr<Expression> Value;

		Add(std::unique_ptr<Expression> Target, std::unique_ptr<Expression> Value) : Target(std::move(Target)), Value(std::move(Value)) {}

		llvm::Value* codegen() override;
	};

	struct Sub : public Expression
	{
		std::unique_ptr<Expression> Target;
		std::unique_ptr<Expression> Value;

		Sub(std::unique_ptr<Expression> Target, std::unique_ptr<Expression> Value) : Target(std::move(Target)), Value(std::move(Value)) {}

		llvm::Value* codegen() override;
	};

	struct Link : public Expression
	{
		llvm::Type* getType = nullptr;
		std::unique_ptr<Expression> Target;
		std::unique_ptr<Expression> Value;

		Link(std::unique_ptr<Expression> Target, std::unique_ptr<Expression> Value) : Target(std::move(Target)), Value(std::move(Value)) {}

		llvm::Value* codegen() override;
	};

	struct VerifyOne : public Expression
	{
		std::unique_ptr<Expression> Target;

		VerifyOne(std::unique_ptr<Expression> Target) : Target(std::move(Target)) {}

		llvm::Value* codegen() override;
	};

	struct Nothing : public Expression
	{
		llvm::Value* codegen() override;
	};

	struct Prototype 
	{
		std::unique_ptr<AST::Type> PType;
		std::string Name;
		std::string TypeAsString;
		std::vector<std::unique_ptr<AST::Variable>> Args;
		
		public:
			Prototype(std::unique_ptr<AST::Type> PType, const std::string &Name, std::vector<std::unique_ptr<AST::Variable>> Args, const std::string &type_as_string)
		 	 : PType(std::move(PType)), Name(Name), Args(std::move(Args)), TypeAsString(type_as_string) {}
		
		const std::string &getName() const { return Name; }

		static std::unique_ptr<Prototype> Error(std::string str)
		{
			ExprError(str);
			return nullptr;
		}

		llvm::Function* codegen();
	};

	static std::map<std::string, std::unique_ptr<AST::Prototype>> FunctionProtos;

	struct Function
	{
		std::unique_ptr<Prototype> Proto;
		std::vector<std::unique_ptr<Expression>> Body;

		Function(std::unique_ptr<Prototype> Proto,
			std::vector<std::unique_ptr<Expression>> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}

		llvm::Function* codegen();
	};
};

#endif