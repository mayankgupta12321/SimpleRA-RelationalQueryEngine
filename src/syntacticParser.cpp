#include "global.h"

bool syntacticParse()
{
    logger.log("syntacticParse");
    string possibleQueryType = tokenizedQuery[0];
    string possibleQueryType_2 = tokenizedQuery[1];

    if (tokenizedQuery.size() < 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    if (possibleQueryType == "CLEAR")
        return syntacticParseCLEAR();
    else if (possibleQueryType == "INDEX")
        return syntacticParseINDEX();
    else if (possibleQueryType == "LIST")
        return syntacticParseLIST();
    else if (possibleQueryType == "LOAD" && possibleQueryType_2 == "MATRIX")
        return syntacticParseLOAD_MATRIX();
    else if (possibleQueryType == "LOAD")
        return syntacticParseLOAD();
    else if (possibleQueryType == "PRINT" && possibleQueryType_2 == "MATRIX")
        return syntacticParsePRINT_MATRIX();
    else if (possibleQueryType == "PRINT")
        return syntacticParsePRINT();
    else if (possibleQueryType == "RENAME" && possibleQueryType_2 == "MATRIX")
        return syntacticParseRENAME_MATRIX();
    else if (possibleQueryType == "RENAME")
        return syntacticParseRENAME();
    else if (possibleQueryType == "EXPORT" && possibleQueryType_2 == "MATRIX")
        return syntacticParseEXPORT_MATRIX();
    else if(possibleQueryType == "EXPORT")
        return syntacticParseEXPORT();
    else if(possibleQueryType == "SOURCE")
        return syntacticParseSOURCE();
    else if (possibleQueryType == "TRANSPOSE" && possibleQueryType_2 == "MATRIX")
        return syntacticParseTRANSPOSE_MATRIX();
    else if(possibleQueryType == "CHECKSYMMETRY")
        return syntacticParseCHECKSYMMETRY();
    else if(possibleQueryType == "COMPUTE")
        return syntacticParseCOMPUTE();
    else if (possibleQueryType == "SORT")
        return syntacticParseSORT();
    else
    {
        string resultantRelationName = possibleQueryType;
        if (tokenizedQuery[1] != "<-" || tokenizedQuery.size() < 3)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        possibleQueryType = tokenizedQuery[2];
        if (possibleQueryType == "PROJECT")
            return syntacticParsePROJECTION();
        else if (possibleQueryType == "SELECT")
            return syntacticParseSELECTION();
        else if (possibleQueryType == "JOIN")
            return syntacticParseJOIN();
        else if (possibleQueryType == "CROSS")
            return syntacticParseCROSS();
        else if (possibleQueryType == "DISTINCT")
            return syntacticParseDISTINCT();
        else if (possibleQueryType == "ORDER")
            return syntacticParseORDER();
        else if (possibleQueryType == "GROUP")
            return syntacticParseGROUP();
        // else if (possibleQueryType == "SORT")
        //     return syntacticParseSORT();
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    return false;
}

ParsedQuery::ParsedQuery()
{
}

void ParsedQuery::clear()
{
    logger.log("ParseQuery::clear");
    this->queryType = UNDETERMINED;

    this->clearRelationName = "";

    this->crossResultRelationName = "";
    this->crossFirstRelationName = "";
    this->crossSecondRelationName = "";

    this->distinctResultRelationName = "";
    this->distinctRelationName = "";

    this->exportRelationName = "";

    this->indexingStrategy = NOTHING;
    this->indexColumnName = "";
    this->indexRelationName = "";

    this->joinBinaryOperator = NO_BINOP_CLAUSE;
    this->joinResultRelationName = "";
    this->joinFirstRelationName = "";
    this->joinSecondRelationName = "";
    this->joinFirstColumnName = "";
    this->joinSecondColumnName = "";

    this->loadRelationName = "";

    this->printRelationName = "";

    this->projectionResultRelationName = "";
    this->projectionColumnList.clear();
    this->projectionRelationName = "";

    this->renameFromColumnName = "";
    this->renameToColumnName = "";
    this->renameRelationName = "";

    this->renameFromMatrixName = "";
    this->renameToMatrixName = "";

    this->selectType = NO_SELECT_CLAUSE;
    this->selectionBinaryOperator = NO_BINOP_CLAUSE;
    this->selectionResultRelationName = "";
    this->selectionRelationName = "";
    this->selectionFirstColumnName = "";
    this->selectionSecondColumnName = "";
    this->selectionIntLiteral = 0;

    this->sortRelationName = "";
    this->sortColumnList.clear();
    this->sortStrategyList.clear();

    this->orderSortingStrategy = NO_SORT_CLAUSE;
    this->orderResultRelationName = "";
    this->orderRelationName = "";
    this->orderColumnName = "";

    this->groupResultRelationName = "";
    this->groupRelationName = "";
    this->groupColumnName = "";
    this->groupBinaryOperator = NO_BINOP_CLAUSE;
    this->groupAggregateColumnValue = 0;
    this->groupAggregateFunction = "";
    this->groupAggregateColumnName = "";
    this->groupReturnAggregateFunction = "";
    this->groupReturnAggregateColumnName = "";

    this->sourceFileName = "";
}

/**
 * @brief Checks to see if source file exists. Called when LOAD command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isFileExists(string tableName)
{
    string fileName = "../data/" + tableName + ".csv";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

/**
 * @brief Checks to see if source file exists. Called when SOURCE command is
 * invoked.
 *
 * @param tableName 
 * @return true 
 * @return false 
 */
bool isQueryFile(string fileName){
    fileName = "../data/" + fileName + ".ra";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}
