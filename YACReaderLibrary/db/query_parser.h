#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "lexertl/generator.hpp"
#include "lexertl/iterator.hpp"

#include <map>
#include <QSqlQuery>
#include <string>
#include <vector>

class QSqlQuery;

class QueryParser {
public:

    enum class TokenType {eof, opcode, atWord, word, quotedWord, space};

    struct TreeNode {
        std::string t;
        std::vector<TreeNode> children;

        int buildSqlString(std::string& sqlString, int bindPosition = 0) const;
        int bindValues(QSqlQuery& selectQuery, int bindPosition = 0) const;
    };

    explicit QueryParser();
    TreeNode parse(const std::string& expr);

private:
    static std::string toLower(const std::string& string);

    std::string token(bool advance = false);
    std::string lcaseToken(bool advance = false);
    TokenType tokenType();
    bool isEof() const;
    void advance();

    template<typename First, typename ... T>
    static bool isIn(First &&first, T && ... t) {return ((first == t) || ...);}

    enum class FieldType {unknown, numeric, text, boolean, date, folder, filename};
    static FieldType fieldType(const std::string& str);

    void tokenize (const std::string& expr);
    static std::string join(const std::vector<std::string>& strings, const std::string& delim);
    static std::vector<std::string> split(const std::string& string, char delim);

    TreeNode orExpression();
    TreeNode andExpression();
    TreeNode notExpression();
    TreeNode locationExpression();
    TreeNode baseToken();

    lexertl::rules lexScanner;
    lexertl::state_machine sm;
    lexertl::siterator iter;
    const lexertl::siterator end;

    static const std::map<FieldType, std::vector<std::string>> fieldNames;
};

#endif // QUERY_PARSER_H
