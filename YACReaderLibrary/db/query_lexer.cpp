#include "query_lexer.h"

QueryLexer::QueryLexer(const std::string &input)
    : input(input)
{
}

Token QueryLexer::next()
{
    while (isSpace(peek())) {
        get();
    }

    switch (peek()) {
    case '\0':
        return Token(Token::Type::eof);
    case '(':
    case ')':
        return single(Token::Type::opcode);
    case ':':
        return single(Token::Type::equal);
    case '=':
        return equal();
    case '<':
        return minor();
    case '>':
        return major();
    case '"':
        return quotedWord();
    default:
        return word();
    }
}

char QueryLexer::peek()
{
    return input[index];
}

char QueryLexer::get()
{
    return input[index++];
}

Token QueryLexer::single(Token::Type type)
{
    return Token(type, input.substr(index++, 1));
}

Token QueryLexer::word()
{
    auto start = index;
    get();
    auto current = peek();
    while (current != '\0' && !isSpace(current) && current != '"' && current != '(' && current != ')' && current != ':' && current != '=' && current != '<' && current != '>') {
        get();
        current = peek();
    }
    return Token(Token::Type::word, input.substr(start, index - start));
}

Token QueryLexer::quotedWord()
{
    auto start = index;
    get();
    auto current = peek();
    while (current != '\0' && current != '"') {
        get();
        current = peek();
    }

    if (current == '"') {
        get();
        return Token(Token::Type::quotedWord, input.substr(start, index - start));
    }

    // This should be a lexical error, but the grammar doesn't support it
    return Token(Token::Type::eof);
}

Token QueryLexer::minor()
{
    auto start = index;
    get();
    auto current = peek();
    if (current == '=') {
        get();
        return Token(Token::Type::minorOrEqual, input.substr(start, index - start));
    }

    return Token(Token::Type::minor, input.substr(start, index - start));
}

Token QueryLexer::major()
{
    auto start = index;
    get();
    auto current = peek();
    if (current == '=') {
        get();
        return Token(Token::Type::majorOrEqual, input.substr(start, index - start));
    }

    return Token(Token::Type::major, input.substr(start, index - start));
}

Token QueryLexer::equal()
{
    auto start = index;
    get();
    auto current = peek();
    if (current == '=') {
        get();
        return Token(Token::Type::exactEqual, input.substr(start, index - start));
    }

    return Token(Token::Type::equal, input.substr(start, index - start));
}

bool QueryLexer::isSpace(char c)
{
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;
    default:
        return false;
    }
}
