#include "query_parser.h"

#include <QVariant>
#include <type_traits>
#include <numeric>
#include <stdexcept>

#include <QsLog.h>

const std::map<QueryParser::FieldType, std::vector<std::string>> QueryParser::fieldNames {
    { FieldType::numeric, { "numpages", "count", "arccount", "alternateCount", "rating" } },
    { FieldType::text, { "date", "number", "arcnumber", "title", "volume", "storyarc", "genere", "writer", "penciller", "inker", "colorist", "letterer", "coverartist", "publisher", "format", "agerating", "synopsis", "characters", "notes", "editor", "imprint", "teams", "locations", "series", "alternateSeries", "alternateNumber", "languageISO", "seriesGroup", "mainCharacterOrTeam", "review", "tags" } },
    { FieldType::boolean, { "color", "read", "edited", "hasBeenOpened" } },
    { FieldType::date, { "added", "lastTimeOpened" } },
    { FieldType::dateFolder, { "added", "updated" } },
    { FieldType::filename, { "filename" } },
    { FieldType::folder, { "folder" } },
    { FieldType::booleanFolder, { "completed", "finished" } },
    { FieldType::enumField, { "type" } },
    { FieldType::enumFieldFolder, { "foldertype" } }
};

std::string operatorToSQLOperator(const std::string &expOperator)
{
    if (expOperator == ":" || expOperator == "=" || expOperator == "==") {
        return "=";
    } else {
        return expOperator;
    }
}

// this parses N days to date N days ago for now
std::string parseDate(const std::string &dateString, const std::string &expOperator)
{
    // TODO_METADATA add semantics so different formats and meanings are supported,
    auto now = QDateTime::currentSecsSinceEpoch();
    auto date = now - stoi(dateString) * 86400;

    return std::to_string(date);
}

int QueryParser::TreeNode::buildSqlString(std::string &sqlString, int bindPosition) const
{
    // TODO: add some semantic checks, not all operators apply to all fields
    if (t == "expression") {
        ++bindPosition;
        if (toLower(children[0].t) == "all") {
            sqlString += "(";
            for (const auto &field : fieldNames.at(FieldType::text)) {
                sqlString += "UPPER(ci." + field + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ") OR ";
            }
            sqlString += "UPPER(c.filename) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ") OR ";
            sqlString += "UPPER(f.name) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (isIn(fieldType(children[0].t), { FieldType::numeric, FieldType::date })) {
            sqlString += "ci." + children[0].t + " " + operatorToSQLOperator(expOperator) + " :bindPosition" + std::to_string(bindPosition) + " ";
        } else if (isIn(fieldType(children[0].t), { FieldType::dateFolder })) {
            sqlString += "f." + children[0].t + " " + operatorToSQLOperator(expOperator) + " :bindPosition" + std::to_string(bindPosition) + " ";
        } else if (isIn(fieldType(children[0].t), { FieldType::boolean, FieldType::enumField })) {
            sqlString += "ci." + children[0].t + " = :bindPosition" + std::to_string(bindPosition) + " ";
        } else if (fieldType(children[0].t) == FieldType::filename) {
            sqlString += "(UPPER(c." + children[0].t + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (fieldType(children[0].t) == FieldType::folder) {
            sqlString += "(UPPER(f.name) LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
        } else if (fieldType(children[0].t) == FieldType::booleanFolder) {
            sqlString += "f." + children[0].t + " = :bindPosition" + std::to_string(bindPosition) + " ";
        } else if (fieldType(children[0].t) == FieldType::enumFieldFolder) {
            if (children[0].t == "foldertype") {
                sqlString += "f.type = :bindPosition" + std::to_string(bindPosition) + " ";
            } else {
                sqlString += "f." + children[0].t + " = :bindPosition" + std::to_string(bindPosition) + " ";
            }
        } else {
            if (expOperator == "=" || expOperator == ":" || expOperator == "") {
                sqlString += "(UPPER(ci." + children[0].t + ") LIKE UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
            } else {
                if (expOperator == "==") {
                    sqlString += "(UPPER(ci." + children[0].t + ") = UPPER(:bindPosition" + std::to_string(bindPosition) + ")) ";
                } else {
                    // support for <,>,<=,>= in text fields makes sense for number, arcNumber, alternateNumber, but (TODO) the syntax won't prevent other fields from using this operators
                    sqlString += "(CAST(ci." + children[0].t + " as REAL) " + expOperator + " CAST(:bindPosition" + std::to_string(bindPosition) + " as REAL)) ";
                }
            }
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
    if (t == "expression") {
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
        } else if ((isIn(fieldType(children[0].t), { FieldType::enumField, FieldType::enumFieldFolder }))) {
            auto enumType = children[0].t;
            auto value = toLower(children[1].t);
            if (enumType == "type" || enumType == "foldertype") {
                if (value == "comic") {
                    selectQuery.bindValue(QString::fromStdString(bind_string), 0);
                } else if (value == "manga") {
                    selectQuery.bindValue(QString::fromStdString(bind_string), 1);
                } else if (value == "westernmanga") {
                    selectQuery.bindValue(QString::fromStdString(bind_string), 2);
                } else if (value == "webcomic" || value == "web") {
                    selectQuery.bindValue(QString::fromStdString(bind_string), 3);
                } else if (value == "4koma" || value == "yonkoma") {
                    selectQuery.bindValue(QString::fromStdString(bind_string), 4);
                }
            } else {
                selectQuery.bindValue(QString::fromStdString(bind_string), std::stoi(children[1].t));
            }
        } else if ((isIn(fieldType(children[0].t), { FieldType::date, FieldType::dateFolder }))) {
            selectQuery.bindValue(QString::fromStdString(bind_string), QString::fromStdString(parseDate(children[1].t, expOperator)));
        } else {
            if (expOperator == "=" || expOperator == ":" || expOperator == "") {
                selectQuery.bindValue(QString::fromStdString(bind_string), QString::fromStdString("%%" + children[1].t + "%%"));
            } else {
                selectQuery.bindValue(QString::fromStdString(bind_string), QString::fromStdString(children[1].t));
            }
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

bool QueryParser::isOperatorToken(Token::Type type)
{
    return type == Token::Type::equal ||
            type == Token::Type::exactEqual ||
            type == Token::Type::minor ||
            type == Token::Type::minorOrEqual ||
            type == Token::Type::major ||
            type == Token::Type::majorOrEqual;
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

    return expression();
}

QueryParser::TreeNode QueryParser::expression()
{
    if (tokenType() == Token::Type::word) {
        auto left = token(true);
        if (isOperatorToken(tokenType())) {
            auto expOperator = token(true);
            if (tokenType() != Token::Type::word && tokenType() != Token::Type::quotedWord) {
                throw std::invalid_argument("missing right operand");
            }
            auto right = token(true);

            return TreeNode("expression", { TreeNode(toLower(left), {}), TreeNode(right, {}) }, expOperator);
        } else {
            return TreeNode("expression", { TreeNode("all", {}), TreeNode(left, {}) });
        }
    }

    return baseToken();
}

QueryParser::TreeNode QueryParser::baseToken()
{
    if (tokenType() == Token::Type::quotedWord) {
        return TreeNode("expression", { TreeNode("all", {}), TreeNode(token(true), {}) });
    }

    if (tokenType() == Token::Type::word) {
        return TreeNode("expression", { TreeNode("all", {}), TreeNode(token(true), {}) });
    }

    return TreeNode("expression", { TreeNode("all", {}), TreeNode(token(true), {}) });
}
