#include "query_parser.h"

#include <QVariant>
#include <sstream>
#include <type_traits>
#include <numeric>

const std::map<QueryParser::FieldType, std::vector<std::string>> QueryParser::fieldNames {
    { FieldType::numeric, { "numpages", "number", "count", "arcnumber", "arccount" } },
    { FieldType::text, { "title", "volume", "storyarc", "genere", "writer", "penciller", "inker", "colorist", "letterer", "coverartist", "publisher", "format", "agerating", "synopsis", "characters", "notes" } },
    { FieldType::boolean, { "isbis", "color" } },
    { FieldType::date, { "date" } },
    { FieldType::filename, { "filename" } },
    { FieldType::folder, { "folder" } }
};

int QueryParser::TreeNode::buildSqlString(std::string &sqlString, int bindPosition) const
{
    if (t == "token") {
        ++bindPosition;
        std::ostringstream oss;
        if (toLower(children[0].t) == "all") {
            oss << "(";
            for (const auto &field : fieldNames.at(FieldType::text)) {
                oss << "UPPER(ci." << field << ") LIKE UPPER(:bindPosition" << bindPosition << ") OR ";
            }
            oss << "UPPER(c.filename) LIKE UPPER(:bindPosition" << bindPosition << ") OR ";
            oss << "UPPER(f.name) LIKE UPPER(:bindPosition" << bindPosition << ")) ";
        } else if (isIn(fieldType(children[0].t), FieldType::numeric, FieldType::boolean)) {
            oss << "ci." << children[0].t << " = :bindPosition" << bindPosition << " ";
        } else if (fieldType(children[0].t) == FieldType::filename) {
            oss << "(UPPER(c." << children[0].t << ") LIKE UPPER(:bindPosition" << bindPosition << ")) ";
        } else if (fieldType(children[0].t) == FieldType::folder) {
            oss << "(UPPER(f.name) LIKE UPPER(:bindPosition" << bindPosition << ")) ";
        } else {
            oss << "(UPPER(ci." << children[0].t << ") LIKE UPPER(:bindPosition" << bindPosition << ")) ";
        }
        sqlString += oss.str();
    } else if (t == "not") {
        sqlString += "(NOT ";
        bindPosition = children[0].buildSqlString(sqlString, bindPosition);
        sqlString += ")";
    } else {
        sqlString += "(";
        bindPosition = children[0].buildSqlString(sqlString, bindPosition);
        sqlString += " " + t + " ";
        bindPosition = children[1].buildSqlString(sqlString, bindPosition);
        sqlString += ")";
    }

    return bindPosition;
}

int QueryParser::TreeNode::bindValues(QSqlQuery &selectQuery, int bindPosition) const
{
    if (t == "token") {
        std::ostringstream oss;
        oss << ":bindPosition" << ++bindPosition;
        if (isIn(fieldType(children[0].t), FieldType::numeric, FieldType::boolean)) {
            selectQuery.bindValue(oss.str().c_str(), std::stoi(children[1].t));
        } else {
            selectQuery.bindValue(oss.str().c_str(), ("%%" + children[1].t + "%%").c_str());
        }
    } else if (t == "not") {
        bindPosition = children[0].bindValues(selectQuery, bindPosition);
    } else {
        bindPosition = children[0].bindValues(selectQuery, bindPosition);
        bindPosition = children[1].bindValues(selectQuery, bindPosition);
    }

    return bindPosition;
}

QueryParser::QueryParser()
    : lexScanner(0)
{

    lexScanner.push("[()]", static_cast<std::underlying_type<TokenType>::type>(TokenType::opcode));
    lexScanner.push("@[^:]+:[^\\\")\\s]+", static_cast<std::underlying_type<TokenType>::type>(TokenType::atWord));
    lexScanner.push("[^\\\"()\\s]+", static_cast<std::underlying_type<TokenType>::type>(TokenType::word));
    lexScanner.push("\\\".*?\\\"", static_cast<std::underlying_type<TokenType>::type>(TokenType::quotedWord));
    lexScanner.push("\\s+", static_cast<std::underlying_type<TokenType>::type>(TokenType::space));

    lexertl::generator::build(lexScanner, sm);
}

QueryParser::TreeNode QueryParser::parse(const std::string &expr)
{
    tokenize(expr);
    auto prog = orExpression();

    if (!isEof()) {
        throw std::invalid_argument("Extra characters at end of search");
    }

    return prog;
}

std::string QueryParser::toLower(const std::string &string)
{
    std::string res(string);
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

std::string QueryParser::token(bool advance)
{
    if (isEof()) {
        return "";
    }
    auto res = (tokenType() == TokenType::quotedWord) ? iter->substr(1, 1) : iter->str();
    if (advance) {
        this->advance();
    }
    return res;
}

std::string QueryParser::lcaseToken(bool advance)
{
    if (isEof()) {
        return "";
    }
    auto res = (tokenType() == TokenType::quotedWord) ? iter->substr(1, 1) : iter->str();
    if (advance) {
        this->advance();
    }
    return toLower(res);
}

QueryParser::TokenType QueryParser::tokenType()
{
    if (isEof()) {
        return TokenType::eof;
    }
    return TokenType(iter->id);
}

bool QueryParser::isEof() const
{
    return iter == end;
}

void QueryParser::advance()
{
    ++iter;
    if (tokenType() == TokenType::space)
        advance();
}

QueryParser::FieldType QueryParser::fieldType(const std::string &str)
{
    for (const auto &names : fieldNames) {
        if (std::find(names.second.begin(), names.second.end(), toLower(str)) != names.second.end()) {
            return names.first;
        }
    }

    return FieldType::unknown;
}

void QueryParser::tokenize(const std::string &expr)
{
    iter = lexertl::siterator(expr.begin(), expr.end(), sm);
}

std::string QueryParser::join(const std::vector<std::string> &strings, const std::string &delim)
{
    return std::accumulate(strings.begin(), strings.end(), std::string(),
                           [&delim](const std::string &a, const std::string &b) -> std::string {
                               return a + (a.length() > 0 && b.length() > 0 ? delim : "") + b;
                           });
}

std::vector<std::string> QueryParser::split(const std::string &string, char delim)
{
    std::istringstream iss(string);
    std::vector<std::string> words;
    while (iss) {
        std::string substr;
        std::getline(iss, substr, delim);
        words.push_back(substr);
    }
    return words;
}

QueryParser::TreeNode QueryParser::orExpression()
{
    auto lhs = andExpression();
    if (lcaseToken() == "or") {
        advance();
        return { "or", { lhs, orExpression() } };
    }
    return lhs;
}

QueryParser::TreeNode QueryParser::andExpression()
{
    auto lhs = notExpression();
    if (lcaseToken() == "and") {
        advance();
        return { "and", { lhs, andExpression() } };
    }

    if ((isIn(tokenType(), TokenType::atWord, TokenType::word, TokenType::quotedWord) || token() == "(") && lcaseToken() != "or") {
        return { "and", { lhs, andExpression() } };
    }

    return lhs;
}

QueryParser::TreeNode QueryParser::notExpression()
{
    if (lcaseToken() == "not") {
        advance();
        return { "not", { notExpression() } };
    }
    return locationExpression();
}

QueryParser::TreeNode QueryParser::locationExpression()
{
    if (tokenType() == TokenType::opcode && token() == "(") {
        advance();
        auto res = orExpression();
        if (tokenType() != TokenType::opcode || token(true) != ")") {
            throw std::invalid_argument("missing )'");
        }
        return res;
    }
    if (!isIn(tokenType(), TokenType::atWord, TokenType::word, TokenType::quotedWord)) {
        throw std::invalid_argument("Invalid syntax. Expected a lookup name or a word");
    }
    return baseToken();
}

QueryParser::TreeNode QueryParser::baseToken()
{
    if (tokenType() == TokenType::quotedWord) {
        return { "token", { { "all", {} }, { token(true), {} } } };
    }

    auto words(split(token(true), ':'));

    if (words.size() > 1 && fieldType(words[0]) != FieldType::unknown) {
        auto loc(toLower(words[0]));
        words.erase(words.begin());
        if (words.size() == 1 && tokenType() == TokenType::quotedWord) {
            return { "token", { { loc, {} }, { token(true), {} } } };
        }
        return { "token", { { loc, {} }, { join(words, ":"), {} } } };
    }
    return { "token", { { "all", {} }, { join(words, ":"), {} } } };
}
