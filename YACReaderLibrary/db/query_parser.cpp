#include "query_parser.h"

#include <QVariant>
#include <type_traits>
#include <numeric>
#include <stdexcept>

const std::map<QueryParser::FieldType, std::vector<std::string>> QueryParser::fieldNames {
    { FieldType::numeric, { "numpages", "number", "count", "arcnumber", "arccount" } },
    { FieldType::text, { "title", "volume", "storyarc", "genere", "writer", "penciller", "inker", "colorist", "letterer", "coverartist", "publisher", "format", "agerating", "synopsis", "characters", "notes" } },
    { FieldType::boolean, { "isbis", "color", "read", "manga" } },
    { FieldType::date, { "date" } },
    { FieldType::filename, { "filename" } },
    { FieldType::folder, { "folder" } },
    { FieldType::booleanFolder, { "completed", "finished" } },
};

int QueryParser::TreeNode::buildSqlString(std::string &sqlString, int bindPosition) const
{
    if (t == "token") {
        ++bindPosition;
        if (toLower(children[0].t) == "all") {
            sqlString += "(";
            for (const auto &field : fieldNames.at(FieldType::text)) {
                sqlString += "UPPER(ci." + field + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ") OR ";
            }
            sqlString += "UPPER(c.filename) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ") OR ";
            sqlString += "UPPER(f.name) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (isIn(fieldType(children[0].t), { FieldType::numeric, FieldType::boolean })) {
            sqlString += "ci." + children[0].t + " = :bindPosition" + std::to_string(bindPosition) + " ";
        } else if (fieldType(children[0].t) == FieldType::filename) {
            sqlString += "(UPPER(c." + children[0].t + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (fieldType(children[0].t) == FieldType::folder) {
            sqlString += "(UPPER(f.name) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (fieldType(children[0].t) == FieldType::booleanFolder) {
            sqlString += "f." + children[0].t + " = :bindPosition" + std::to_string(bindPosition) + " ";
        } else {
            sqlString += "(UPPER(ci." + children[0].t + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        }
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
        std::string bind_string(":bindPosition" + std::to_string(++bindPosition));
        if (isIn(fieldType(children[0].t), { FieldType::numeric })) {
            selectQuery.bindValue(QString::fromStdString(bind_string), std::stoi(children[1].t));
        } else if (isIn(fieldType(children[0].t), { FieldType::boolean, FieldType::booleanFolder })) {
            auto value = toLower(children[1].t);
            if (value == "true") {
                selectQuery.bindValue(QString::fromStdString(bind_string), 1);
            } else if (value == "false") {
                selectQuery.bindValue(QString::fromStdString(bind_string), 0);
            } else {
                selectQuery.bindValue(QString::fromStdString(bind_string), std::stoi(value));
            }
        } else {
            selectQuery.bindValue(QString::fromStdString(bind_string), QString::fromStdString("%%" + children[1].t + "%%"));
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
{
}

QueryParser::TreeNode QueryParser::parse(const std::string &expr)
{
    lexer = QueryLexer(expr);
    advance();

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

    auto lexeme = currentToken.lexeme();

    auto res = (tokenType() == Token::Type::quotedWord) ? currentToken.lexeme().substr(1, lexeme.size() - 2) : lexeme; // TODO process quotedWordDiferently?
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

    auto lexeme = currentToken.lexeme();

    auto res = (tokenType() == Token::Type::quotedWord) ? currentToken.lexeme().substr(1, lexeme.size() - 2) : lexeme;

    if (advance) {
        this->advance();
    }
    return toLower(res);
}

Token::Type QueryParser::tokenType()
{
    return currentToken.type();
}

bool QueryParser::isEof() const
{
    return currentToken.type() == Token::Type::eof;
}

void QueryParser::advance()
{
    currentToken = lexer.next();
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

std::string QueryParser::join(const QStringList &strings, const std::string &delim)
{
    return std::accumulate(strings.begin(), strings.end(), std::string(),
                           [&delim](const std::string &a, const QString &b) -> std::string {
                               return a + (a.length() > 0 && b.length() > 0 ? delim : "") + b.toStdString();
                           });
}

QStringList QueryParser::split(const std::string &string, char delim)
{
    auto words = QString::fromStdString(string).split(delim);
    return words;
}

QueryParser::TreeNode QueryParser::orExpression()
{
    auto lhs = andExpression();
    if (lcaseToken() == "or") {
        advance();
        return TreeNode("or", { lhs, orExpression() });
    }
    return lhs;
}

QueryParser::TreeNode QueryParser::andExpression()
{
    auto lhs = notExpression();
    if (lcaseToken() == "and") {
        advance();
        return TreeNode("and", { lhs, andExpression() });
    }

    if ((isIn(tokenType(), { Token::Type::word, Token::Type::quotedWord }) || token() == "(") && lcaseToken() != "or") {
        return TreeNode("and", { lhs, andExpression() });
    }

    return lhs;
}

QueryParser::TreeNode QueryParser::notExpression()
{
    if (lcaseToken() == "not") {
        advance();
        return TreeNode("not", { notExpression() });
    }
    return locationExpression();
}

QueryParser::TreeNode QueryParser::locationExpression()
{
    if (tokenType() == Token::Type::opcode && token() == "(") {
        advance();
        auto res = orExpression();
        if (tokenType() != Token::Type::opcode || token(true) != ")") {
            throw std::invalid_argument("missing )'");
        }
        return res;
    }
    if (!isIn(tokenType(), { Token::Type::word, Token::Type::quotedWord })) {
        throw std::invalid_argument("Invalid syntax. Expected a lookup name or a word");
    }
    return baseToken();
}

QueryParser::TreeNode QueryParser::baseToken()
{
    if (tokenType() == Token::Type::quotedWord) {
        return TreeNode("token", { TreeNode("all", {}), TreeNode(token(true), {}) });
    }

    auto words(split(token(true), ':'));

    if (words.size() > 1 && fieldType(words[0].toStdString()) != FieldType::unknown) {
        auto loc(toLower(words[0].toStdString()));
        words.erase(words.begin());
        if (words.size() == 1 && tokenType() == Token::Type::quotedWord) {
            return TreeNode("token", { TreeNode(loc, {}), TreeNode(token(true), {}) });
        }
        return TreeNode("token", { TreeNode(loc, {}), TreeNode(join(words, ":"), {}) });
    }

    return TreeNode("token", { TreeNode("all", {}), TreeNode(join(words, ":"), {}) });
}
