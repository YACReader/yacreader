#ifndef QUERY_LEXER_H
#define QUERY_LEXER_H

#include <string>

class Token
{
public:
    enum class Type {
        eof,
        opcode,
        word,
        quotedWord,
        undefined
    };

    Token(Type type, std::string lexeme = "")
        : _type(type), _lexeme(std::move(lexeme))
    {
    }

    Type type() const
    {
        return _type;
    }

    std::string lexeme() const
    {
        return _lexeme;
    }

private:
    Type _type {};
    std::string _lexeme {};
};

class QueryLexer
{
public:
    QueryLexer(const std::string &input);
    Token next();

private:
    std::string input;
    int index = 0;

    char peek();
    char get();

    Token single(Token::Type type);
    Token word();
    Token quotedWord();

    bool isSpace(char c);
};

#endif // QUERY_LEXER_H
