#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include "lexer.h"
#include <map>

/**************************************************************************
 * AST classes
 **************************************************************************/

class ExprAST{
public:
    virtual ~ExprAST(){}
};

// a number
class NumExprAST: public ExprAST{
private:
    double val;
public:
    NumExprAST(double val): val(val) {}
};

// a variable
class VarExprAST: public ExprAST{
private:
    std::string name;
public:
    VarExprAST(const std::string& name): name(name){}
};

// a binary operator
class BinaryExprAST: public ExprAST{
private:
    char op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS):
        op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

// a function call
class CallExprAST: public ExprAST{
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(const std::string& name, std::vector<std::unique_ptr<ExprAST>> args):
        callee(name), args(std::move(args)) {}
};

//-------------------- a function prototype ----------------------------
class PrototypeAST{
private:
    std::string name;
    std::vector<std::string> args;
public:
    PrototypeAST(std::string name, std::vector<std::string> agrs):
        name(name), args(std::move(args)) {}
    
    const std::string& getName() const{
        return name;
    }
};

//-------------------- a function definition ----------------------------
class FunctionAST{
private:
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body):
        proto(std::move(proto)), body(std::move(body)) {}
};


/**************************************************************************
 * parsers
 **************************************************************************/
 //parser basics
static Token currToken;
static Token nextToken(std::istream &is = std::cin){
    return currToken = getTok(is);
}

static std::unique_ptr<ExprAST> parseExpr();

//------------------------- print error msg------------------------------
template <typename T>
T error(const std::string& msg){
    std::cerr << msg << std::endl;
}


/**************************************************************************
 * primary expression parser
 **************************************************************************/
static std::unique_ptr<ExprAST> parseNum(){
    std::unique_ptr<NumExprAST> result = std::make_unique<NumExprAST>(NumVal);
    nextToken();
    return std::move(result);
}

//----------ignore the parenthesis when an AST is parsed-------------------------------
static std::unique_ptr<ExprAST> parseParen(){
    nextToken();
    std::unique_ptr<ExprAST> result = parseExpr();
    if(!result)
        return nullptr;
    
    if(currToken == Token::tok_operator && Operator == ')')
    {
        nextToken(); 
        return result;
    }

    return error<std::unique_ptr<ExprAST>>("expected )");
}


static std::unique_ptr<ExprAST> parseIdentfier(){
    std::string name = IdentifierStr;
    nextToken();

    if(currToken != Token::tok_operator || Operator != '(')
        return std::make_unique<VarExprAST>(name);
    
    nextToken(); //processed '('
    std::vector<std::unique_ptr<ExprAST>> args;
    if(currToken != Token::tok_operator || Operator != ')')
    {
        while(true)
        {
            // an argument is processed
            if(std::unique_ptr<ExprAST> arg = parseExpr())
                args.push_back(std::move(arg));
            else 
                return nullptr;
            
            if(currToken == Token::tok_operator && Operator == ')')
                break;
            
            if(currToken != Token::tok_operator || Operator != ',')
                return error<std::unique_ptr<ExprAST>>("Expected ')' or ',' in argument list");
            
            nextToken();
        }
    }

    nextToken(); // processed ')'

    return std::make_unique<CallExprAST>(name, std::move(args));
}

static std::unique_ptr<ExprAST> parsePrimary(){
    if(currToken == Token::tok_number)
        return parseNum();
    if(currToken == Token::tok_identifier)
        return parseIdentfier();
    if(currToken == Token::tok_operator && Operator == '(')
        return parseParen();
    return error<std::unique_ptr<ExprAST>>("unknown token when parsing a primary expression");
}


/**************************************************************************
 * binary expression parsers
 **************************************************************************/
//binary parser basics
static std::unique_ptr<ExprAST> parseBinaryRHS(int, std::unique_ptr<ExprAST>);

static std::unique_ptr<ExprAST> parseExpr(){
    std::unique_ptr<ExprAST> LHS = parsePrimary();
    if(!LHS)
        return nullptr;
    
    return parseBinaryRHS(0, std::move(LHS));
}

//binary operator precendence table
static std::map<char, int> binaryPrecedence{
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40}
};

static int getTokenPrecedence(){
    if(currToken != Token::tok_operator)
        return -1;
    
    int tokenPrec = binaryPrecedence[Operator];
    if(tokenPrec <= 0)
        return -1;
    
    return tokenPrec;
}

//binary expressions parser
static std::unique_ptr<ExprAST> parseBinaryRHS(int prevPrec, std::unique_ptr<ExprAST> LHS){
    while(true)
    {
        int currPrec = getTokenPrecedence();

        // if currToken is not a binary operator, it will be returned from here
        if(currPrec < prevPrec)
            return LHS; //the exit point of a completed AST
        
        char currOperator = Operator;
        nextToken();

        std::unique_ptr<ExprAST> RHS = parsePrimary();
        if(!RHS)
        {
            return error<std::unique_ptr<ExprAST>>("Second operand of a binary operator not found");
        }

        int nextPrec = getTokenPrecedence();
        if(currPrec < nextPrec)
        {
            RHS = parseBinaryRHS(currPrec + 1, std::move(RHS));
            if(!RHS)
            {
                return error<std::unique_ptr<ExprAST>>("Second operand of a binary operator not found");
            }
        }

        LHS = std::make_unique<BinaryExprAST>(currOperator, std::move(LHS), std::move(RHS));
    }
}

/**************************************************************************
 * function parsers
 **************************************************************************/
//function prototype(signature) parser
static std::unique_ptr<PrototypeAST> parsePrototype(){
    if(currToken != Token::tok_identifier)
        return error<std::unique_ptr<PrototypeAST>>("expected an identifier for a function");
    
    std::string name = IdentifierStr;
    nextToken();

    if(currToken != Token::tok_operator || Operator != '(')
        return error<std::unique_ptr<PrototypeAST>>("expected a '(' in prototype");
    
    std::vector<std::string> args;
    while(nextToken() == Token::tok_identifier)
        args.push_back(IdentifierStr);
    
    if(currToken != Token::tok_operator || Operator != ')')
        return error<std::unique_ptr<PrototypeAST>>("expected a ')' in prototype");
    
    nextToken();

    return std::make_unique<PrototypeAST>(name, std::move(args));
}

//function definition parser
static std::unique_ptr<FunctionAST> parseDefinition(){
    nextToken();

    std::unique_ptr<PrototypeAST> proto = parsePrototype();
    if(!proto)
        return nullptr;
    
    std::unique_ptr<ExprAST> body = parseExpr();
    if(!body)
        return nullptr;
    
    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

//extern functions declaration (for external library or forward declaration)
static std::unique_ptr<PrototypeAST> parseExtern(){
    nextToken();
    return parsePrototype();
}

//top level expressions parser ????
static std::unique_ptr<FunctionAST> parseTopLevelExpr(){
    if(std::unique_ptr<ExprAST> exp = parseExpr())
    {
        std::unique_ptr<PrototypeAST> proto 
            = std::make_unique<PrototypeAST>("", std::vector<std::string>());
        
        return std::make_unique<FunctionAST>(std::move(proto), std::move(exp));
    }

    return nullptr;
}


















#endif