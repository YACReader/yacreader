#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query_lexer.h"

#include <map>
#include <QSqlQuery>
#include <string>
#include <vector>
#include <list>

/**
 * This class is used to generate an SQL query string from a search expression,
 * with a syntax very similar to that used by the Google search engine.
 *
 * The code herein is based upon the SearchQueryParser python class written by
 * Kovid Goyal as part of the Calibre eBook manager (https://calibre-ebook.com)
 *
 * Grammar:
 *   prog ::= or_expression
 *   or_expression ::= and_expression [ 'or' or_expression ]
 *   and_expression ::= not_expression [ [ 'and' ] and_expression ]
 *   not_expression ::= [ 'not' ] location_expression
 *   location_expression ::= base_token | ( '(' or_expression ')' )
 *   base_token ::= a sequence of letters and colons, perhaps quoted
 *
 * Usage Example:
 *    QSqlQuery selectQuery(db);
 *    std::string queryString("SELECT ... FROM ... WHERE ");
 *
 *    QueryParser parser;                   // Create the parser object
 *    TreeNode result = parser.parse(expr); // Parse the query expression
 *
 *    result.buildSqlString(queryString);   // Append the SQL query to a string
 *
 *    selectQuery.prepare(queryString.c_str()); // Convert the string to a query
 *    result.bindValues(selectQuery);       // Populate the SQL query variables
 *
 *    selectQuery.exec();
 */
class QueryParser
{
public:
    struct TreeNode {
        std::string t;
        std::vector<TreeNode> children;

        explicit TreeNode(std::string t, std::vector<TreeNode> children)
            : t(t), children(children)
        {
        }

        int buildSqlString(std::string &sqlString, int bindPosition = 0) const;
        int bindValues(QSqlQuery &selectQuery, int bindPosition = 0) const;
    };

    explicit QueryParser();
    TreeNode parse(const std::string &expr);

private:
    static std::string toLower(const std::string &string);

    std::string token(bool advance = false);
    std::string lcaseToken(bool advance = false);
    Token::Type tokenType();
    bool isEof() const;
    void advance();

    QueryLexer lexer = QueryLexer("");
    Token currentToken = Token(Token::Type::undefined);

    template<typename T>
    static bool isIn(const T &e, const std::list<T> &v)
    {
        return std::find(v.begin(), v.end(), e) != v.end();
    }

    enum class FieldType { unknown,
                           numeric,
                           text,
                           boolean,
                           date,
                           folder,
                           filename };
    static FieldType fieldType(const std::string &str);

    static std::string join(const QStringList &strings, const std::string &delim);
    static QStringList split(const std::string &string, char delim);

    TreeNode orExpression();
    TreeNode andExpression();
    TreeNode notExpression();
    TreeNode locationExpression();
    TreeNode baseToken();

    static const std::map<FieldType, std::vector<std::string>> fieldNames;
};

#endif // QUERY_PARSER_H
