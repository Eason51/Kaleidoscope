#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

template<typename T>
static void print(T val){
    std::cout << val << std::endl;
}

enum class Token{ 
    tok_eof = -1,           // end of file
    
    tok_def = -2,           // command "def"
    tok_extern = -3,        // command "extern"

    tok_identifier = -4,    // an identifier
    tok_number = -5,        // a number
    tok_operator = -6       // an operator
};

static std::string IdentifierStr;
static double NumVal;
static char Operator;

static Token getTok(std::istream& is){
    static char LastChar = ' ';

    if(is.eof())
        return Token::tok_eof;

    if(LastChar == ' ' || LastChar == '\t' || LastChar == '\n' || LastChar == '\v' ||
        LastChar == '\f' || LastChar == '\r')
        is >> std::skipws >> LastChar;  
    
    // if the first char is [a, z] || [A, Z]
    if((LastChar >= 65 && LastChar <= 90) 
    || (LastChar >= 97 && LastChar <= 122))
    {
        IdentifierStr = LastChar;
        // subsequent chars can be numbers
        while((is >> std::noskipws >> LastChar) && ((LastChar >= 48 && LastChar <= 57)
            || (LastChar >= 65 && LastChar <= 90) 
            || (LastChar >= 97 && LastChar <= 122)))
        {
            IdentifierStr += LastChar;            
        }

        if(IdentifierStr == "def")
            return Token::tok_def;
        if(IdentifierStr == "extern")
            return Token::tok_extern;
        
        return Token::tok_identifier; 
    }
    
    //if the first char is a digit or decimal point
    if((LastChar >= 48 && LastChar <= 57) || LastChar == 46)
    {
        std::string numStr;
        numStr += LastChar;
        while((is >> std::noskipws >> LastChar) &&
            ((LastChar >= 48 && LastChar <= 57) || LastChar == 46))
        {
            numStr += LastChar;
        }
        
        NumVal = strtod(numStr.c_str(), nullptr);
        return Token::tok_number;
    }

    if(LastChar == 35)
    {
        while((is >> std::noskipws >> LastChar)
            &&(LastChar != 10) && (LastChar != 13));

        if(!is.eof())
            return getTok(is);
        if(is.eof())
            return Token::tok_eof;
    }

    if(is.eof())
        return Token::tok_eof;
    
    int ThisChar = LastChar;
    is >> std::noskipws >> LastChar;
    Operator = ThisChar;
    return Token::tok_operator;
}

#endif