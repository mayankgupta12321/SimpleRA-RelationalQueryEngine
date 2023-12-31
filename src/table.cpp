#include "global.h"

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName 
 */
Table::Table(string tableName)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/" + tableName + ".csv";
    this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName 
 * @param columns 
 */
Table::Table(string tableName, vector<string> columns)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded 
 * @return false if an error occurred 
 */
bool Table::load()
{
    logger.log("Table::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file. 
 *
 * @param line 
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine)
{
    logger.log("Table::extractColumnNames");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify()
{
    logger.log("Table::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Table::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName 
 * @return true 
 * @return false 
 */
bool Table::isColumn(string columnName)
{
    logger.log("Table::isColumn");
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName 
 * @param toColumnName 
 */
void Table::renameColumn(string fromColumnName, string toColumnName)
{
    logger.log("Table::renameColumn");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (columns[columnCounter] == fromColumnName)
        {
            columns[columnCounter] = toColumnName;
            break;
        }
    }
    return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print()
{
    logger.log("Table::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings
    this->writeRow(this->columns, cout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}



/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Table::getNextPage(Cursor *cursor)
{
    logger.log("Table::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent()
{
    logger.log("Table::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent()
{
    logger.log("Table::isPermanent");
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
    return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload(){
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 * 
 * @return Cursor 
 */
Cursor Table::getCursor()
{
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
int Table::getColumnIndex(string columnName)
{
    logger.log("Table::getColumnIndex");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}


void Table::sortTable(vector<int> columnIndices, vector<SortingStrategy> sortStrategyList)
{
    logger.log("Table::sortTable");

    // Comparator for Sort Function
    auto cmpSort = [this, columnIndices, sortStrategyList](vector<int> &a, vector<int> &b) {
        for(int i = 0; i < columnIndices.size(); i++) {
            if(sortStrategyList[i] == ASC) {
                if(a[columnIndices[i]] < b[columnIndices[i]]) return true;
                if(a[columnIndices[i]] > b[columnIndices[i]]) return false;
            }
            else {
                if(a[columnIndices[i]] < b[columnIndices[i]]) return false;
                if(a[columnIndices[i]] > b[columnIndices[i]]) return true;
            }
        }
        return false;
    };
    
    // Sorting Phase
    Cursor cursor(tableName, 0);
    for(int i = 0; i < blockCount; i++) {
        vector<vector<int>> rows = cursor.getPage();
        int nRows = rowsPerBlockCount[i];
        sort(rows.begin(), rows.begin() + nRows, cmpSort);
        bufferManager.writePage(tableName, i, rows, nRows);
        bufferManager.deleteFromPool(cursor.page.pageName);
        if(i + 1 < blockCount)  cursor.nextPage(i + 1);
    }


    // Merge Phase
    int degreeOfMerge = BLOCK_COUNT - 1;
    int totalLevels = ceil(log2(blockCount) / log2(degreeOfMerge));


    // Comparator for Priority Queue
    auto cmpPQ = [this, columnIndices, sortStrategyList](pair<vector<int>, int> &a, pair<vector<int>, int> &b) {
        for(int i = 0; i < columnIndices.size(); i++) {
            if(sortStrategyList[i] == ASC) {
                if(a.first[columnIndices[i]] > b.first[columnIndices[i]]) return true;
                if(a.first[columnIndices[i]] < b.first[columnIndices[i]]) return false;
            }
            else {
                if(a.first[columnIndices[i]] > b.first[columnIndices[i]]) return false;
                if(a.first[columnIndices[i]] < b.first[columnIndices[i]]) return true;
            }
        }
        return true;
    };

    vector<uint> prefix = {0};
    for(int c: rowsPerBlockCount) {
        prefix.push_back(prefix.back() + c);  
    }


    for(int level = 0; level < totalLevels; level++) {
        int tempPageIndex = 0;
        for(int i = 0; i < blockCount; i += pow(3, level + 1)) {

            priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, decltype(cmpPQ)> pq(cmpPQ);
            
            vector<Cursor> cursorPool;
            vector<int> recordsToProcessed;
            int idx = 0;

            int jump = pow(3, level);
            for(int j = i; j < i + pow(3, level + 1) && j < blockCount; j += jump) {
                int left = j;
                int right = min(j + jump, (int)blockCount);
                int nRecords = prefix[right] - prefix[left];
                recordsToProcessed.push_back(nRecords);
                // cout << left << " " << right << " " << nRecords << "\n";

                Cursor cursor(this->tableName, j);
                cursorPool.push_back(cursor);
                pq.push({cursorPool[idx].getNext(), idx});
                recordsToProcessed[idx]--;
                idx++;
            }

            vector<vector<int>> rows;
            while(!pq.empty()) {
                auto [row, idx] = pq.top();
                pq.pop();

                rows.push_back(row);

                if(rows.size() == maxRowsPerBlock) {
                    bufferManager.writePage("$sortTemp_" + tableName, tempPageIndex, rows, rows.size());
                    tempPageIndex++;
                    rows.clear();
                }

                if(recordsToProcessed[idx] > 0) {
                    pq.push({cursorPool[idx].getNext(), idx});
                    recordsToProcessed[idx]--;
                }
            }

            if(!rows.empty()) {
                bufferManager.writePage("$sortTemp_" + tableName, tempPageIndex, rows, rows.size());
                tempPageIndex++;
                rows.clear();
            }
        }
        for(int i = 0; i < blockCount; i++) {
            bufferManager.renameFile("$sortTemp_" + tableName, tableName, i);
        }
    }
}


void Table::groupTable(Table* tempTable, int groupColumnIndex, BinaryOperator groupBinaryOperator, int groupAggregateColumnValue, string groupAggregateFunction, int groupAggregateColumnIndex, string groupReturnAggregateFunction, int groupReturnAggregateColumnIndex) {
    logger.log("Table::groupTable");
    
    this->distinctValuesInColumns.assign(this->columnCount, {});
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    
    vector<vector<int>> rows;

    Cursor cursor(tempTable->tableName, 0);
    vector<int> row = cursor.getNext();

    int prevColumnVal = row[groupColumnIndex];
    int rowCount = 0;
    int groupAggregateColumnRes = (groupAggregateFunction == "MIN") ? 1000 : 0;
    int groupReturnAggregateColumnRes = (groupReturnAggregateFunction == "MIN") ? 1000 : 0;

    while(!row.empty()) {
        if(row[groupColumnIndex] != prevColumnVal) {
            if(groupAggregateFunction == "AVG") {
                groupAggregateColumnRes /= rowCount;
            }

            if(groupReturnAggregateFunction == "AVG") {
                groupReturnAggregateColumnRes /= rowCount;
            }

            if(groupBinaryOperator == LESS_THAN) {
                if(groupAggregateColumnRes < groupAggregateColumnValue) {
                    rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
                    this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
                }
            }
            else if(groupBinaryOperator == LEQ) {
                if(groupAggregateColumnRes <= groupAggregateColumnValue) {
                    rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
                    this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
                }
            }
            else if(groupBinaryOperator == GREATER_THAN) {
                if(groupAggregateColumnRes > groupAggregateColumnValue) {
                    rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
                    this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
                }
            }
            else if(groupBinaryOperator == GEQ) {
                if(groupAggregateColumnRes >= groupAggregateColumnValue) {
                    rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
                    this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
                }
            }
            else {
                if(groupAggregateColumnRes == groupAggregateColumnValue) {
                    rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
                    this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
                }
            }

            if(rows.size() == this->maxRowsPerBlock) {
                bufferManager.writePage(this->tableName, this->blockCount, rows, rows.size());
                this->blockCount++;
                this->rowsPerBlockCount.emplace_back(rows.size());
                rows.clear();
            }
            
            prevColumnVal = row[groupColumnIndex];
            rowCount = 0;
            groupAggregateColumnRes = (groupAggregateFunction == "MIN") ? 1000 : 0;
            groupReturnAggregateColumnRes = (groupReturnAggregateFunction == "MIN") ? 1000 : 0;
        }

        if(groupAggregateFunction == "MAX") {
            groupAggregateColumnRes = max(groupAggregateColumnRes, row[groupAggregateColumnIndex]);
        }
        else if(groupAggregateFunction == "MIN") {
            groupAggregateColumnRes = min(groupAggregateColumnRes, row[groupAggregateColumnIndex]);
        }
        else {
            groupAggregateColumnRes += row[groupAggregateColumnIndex];
        }

        if(groupReturnAggregateFunction == "MAX") {
            groupReturnAggregateColumnRes = max(groupReturnAggregateColumnRes, row[groupReturnAggregateColumnIndex]);
        }
        else if(groupReturnAggregateFunction == "MIN") {
            groupReturnAggregateColumnRes = min(groupReturnAggregateColumnRes, row[groupReturnAggregateColumnIndex]);
        }
        else {
            groupReturnAggregateColumnRes += row[groupReturnAggregateColumnIndex];
        }
        
        rowCount++;

        row = cursor.getNext();
    }

    if(groupAggregateFunction == "AVG") {
        groupAggregateColumnRes /= rowCount;
    }

    if(groupReturnAggregateFunction == "AVG") {
        groupReturnAggregateColumnRes /= rowCount;
    }
            
    if(groupBinaryOperator == LESS_THAN) {
        if(groupAggregateColumnRes < groupAggregateColumnValue) {
            rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
            this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
        }
    }
    else if(groupBinaryOperator == LEQ) {
        if(groupAggregateColumnRes <= groupAggregateColumnValue) {
            rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
            this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
        }
    }
    else if(groupBinaryOperator == GREATER_THAN) {
        if(groupAggregateColumnRes > groupAggregateColumnValue) {
            rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
            this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
        }
    }
    else if(groupBinaryOperator == GEQ) {
        if(groupAggregateColumnRes >= groupAggregateColumnValue) {
            rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
            this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
        }
    }
    else {
        if(groupAggregateColumnRes == groupAggregateColumnValue) {
            rows.push_back({prevColumnVal, groupReturnAggregateColumnRes});
            this->updateStatistics({prevColumnVal, groupReturnAggregateColumnRes});
        }
    }
    
    if(rows.size() > 0) {
        bufferManager.writePage(this->tableName, this->blockCount, rows, rows.size());
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(rows.size());
        rows.clear();
    }

}

void Table::insertNewRow(vector<int> &row1, vector<int> &row2, vector<vector<int>> &rows) {
    vector<int> currentRow;
    currentRow.insert(currentRow.end(), row1.begin(), row1.end());
    currentRow.insert(currentRow.end(), row2.begin(), row2.end());
    rows.push_back(currentRow);
    this->updateStatistics(currentRow);

    if(rows.size() == this->maxRowsPerBlock) {
        bufferManager.writePage(this->tableName, this->blockCount, rows, rows.size());
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(rows.size());
        rows.clear();
    }
}

void Table::joinTable(Table *table1, Table *table2, int joinFirstColumnIndex, int joinSecondColumnIndex, BinaryOperator joinBinaryOperator) {
    logger.log("Table::joinTable");
    
    this->distinctValuesInColumns.assign(this->columnCount, {});
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    
    vector<vector<int>> rows;

    Cursor cursor1(table1->tableName, 0);
    Cursor cursor2(table2->tableName, 0);

    vector<int> row1 = cursor1.getNext();
    vector<int> row2 = cursor2.getNext();

    while(!row1.empty() && !row2.empty()) {
        if(joinBinaryOperator == EQUAL) {
            if(row1[joinFirstColumnIndex] == row2[joinSecondColumnIndex]) {
                insertNewRow(row1, row2, rows);

                Cursor nextCursor1 = cursor1, nextCursor2 = cursor2;
                vector<int> nextRow1 = nextCursor1.getNext(), nextRow2 = nextCursor2.getNext();
                while(!nextRow1.empty()) {
                    if(nextRow1[joinFirstColumnIndex] == row2[joinSecondColumnIndex]) {
                        insertNewRow(nextRow1, row2, rows);
                    }
                    else {
                        break;
                    }

                    nextRow1 = nextCursor1.getNext();
                }

                while(!nextRow2.empty()) {
                    if(row1[joinFirstColumnIndex] == nextRow2[joinSecondColumnIndex]) {
                        insertNewRow(row1, nextRow2, rows);
                    }
                    else {
                        break;
                    }

                    nextRow2 = nextCursor2.getNext();
                }

                row1 = cursor1.getNext();
                row2 = cursor2.getNext();
            }
            else if(row1[joinFirstColumnIndex] < row2[joinSecondColumnIndex]) {
                row1 = cursor1.getNext();
            }
            else {
                row2 = cursor2.getNext();
            }
        }
        else if(joinBinaryOperator == GREATER_THAN) {
            if(row1[joinFirstColumnIndex] > row2[joinSecondColumnIndex]) {
                insertNewRow(row1, row2, rows);

                Cursor nextCursor1 = cursor1;
                vector<int> nextRow1 = nextCursor1.getNext();
                while(!nextRow1.empty()) {
                    insertNewRow(nextRow1, row2, rows);
                    nextRow1 = nextCursor1.getNext();
                }

                row2 = cursor2.getNext();
            }
            else {
                row1 = cursor1.getNext();
            }
        }
        else if(joinBinaryOperator == LESS_THAN) {
            if(row1[joinFirstColumnIndex] < row2[joinSecondColumnIndex]) {
                insertNewRow(row1, row2, rows);

                Cursor nextCursor2 = cursor2;
                vector<int> nextRow2 = nextCursor2.getNext();
                while(!nextRow2.empty()) {
                    insertNewRow(row1, nextRow2, rows);
                    nextRow2 = nextCursor2.getNext();
                }

                row1 = cursor1.getNext();
            }
            else {
                row2 = cursor2.getNext();
            }
        }
        else if(joinBinaryOperator == GEQ) {
            if(row1[joinFirstColumnIndex] >= row2[joinSecondColumnIndex]) {
                insertNewRow(row1, row2, rows);

                Cursor nextCursor1 = cursor1;
                vector<int> nextRow1 = nextCursor1.getNext();
                while(!nextRow1.empty()) {
                    insertNewRow(nextRow1, row2, rows);
                    nextRow1 = nextCursor1.getNext();
                }

                row2 = cursor2.getNext();
            }
            else {
                row1 = cursor1.getNext();
            }
        }
        else if(joinBinaryOperator == LEQ) {
            if(row1[joinFirstColumnIndex] <= row2[joinSecondColumnIndex]) {
                insertNewRow(row1, row2, rows);

                Cursor nextCursor2 = cursor2;
                vector<int> nextRow2 = nextCursor2.getNext();
                while(!nextRow2.empty()) {
                    insertNewRow(row1, nextRow2, rows);
                    nextRow2 = nextCursor2.getNext();
                }

                row1 = cursor1.getNext();
            }
            else {
                row2 = cursor2.getNext();
            }
        }
    }

    if(rows.size() > 0) {
        bufferManager.writePage(this->tableName, this->blockCount, rows, rows.size());
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(rows.size());
        rows.clear();
    }
}