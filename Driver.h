#ifndef DRIVER_H
#define DRIVER_H

#include "AST.h"

static void handleDefinition(){
    if(parseDefinition())
        print("parsed a function definition");
    else
        nextToken();    
}

static void handleExtern(){
    if(parseExtern())
        print("parsed an extern declaration");
    else
        nextToken();
}

static void handleTopLevelExpression(){
    if(parseTopLevelExpr())
        print("parsed a top level expression");
    else
        nextToken();
}

//mainloop
static void mainloop(){
    while(true)
    {
        print("ready> \n");
        
        if(currToken == Token::tok_eof)
            return;
        if(currToken == Token::tok_operator && Operator == ';')
        {
            nextToken();
            continue;
        }
        if(currToken == Token::tok_def)
        {
            handleDefinition();
            continue;
        }
        if(currToken == Token::tok_extern)
        {
            handleExtern();
            continue;
        }
        else
        {
            handleTopLevelExpression();
            continue;
        }        
    }
}

#endif