#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

enum class StatementType : int { SELECT, INSERT, DELETE };

// Different numerical values needed to access data in memory/disk
const int KEY_SIZE = 4;
const int KEY_OFFSET = 0;
const int NAME_SIZE = 33;
const int NAME_OFFSET = KEY_SIZE;
const int EMAIL_SIZE = 51;
const int EMAIL_OFFSET = NAME_OFFSET + NAME_SIZE;

const int MAX_PAGES = 50;
const int ENTRY_SIZE = KEY_SIZE + NAME_SIZE + EMAIL_SIZE;
const int PAGE_SIZE = 4096;
const int ENTRIES_PER_PAGE = PAGE_SIZE / ENTRY_SIZE;
const int MAX_ENTRIES = ENTRIES_PER_PAGE * MAX_PAGES;

const string TABLE_FILENAME = "data.bin"; 

// Class Definitions
class Entry {
public:
  uint32_t key;
  char name[33];
  char email[51];
  // 88 bytes total for one entry
  void printSelf();
};

class Statement {
public:
  StatementType type;
  Entry new_entry;
};

class Table {
public:
  uint32_t numEntries;
  void *pages[MAX_PAGES];
  int fd;
  Table();
  ~Table();
  void loadTable();
  void flushPage(uint32_t page_num);
  void* getPagePointer(uint32_t page_num);
};

class Database {
  public:
    void prompt();
    int metaCommand(string &input);
    int parseCommand(string &input, Statement &st);
    int executeStatement(Statement st, Table &table);
    void *entrySlot(Table &table, uint32_t entry_num);
    void deserializeEntry(void *source, Entry &destination);
    void serializeEntry(Entry &source, void *destination);
};

#endif