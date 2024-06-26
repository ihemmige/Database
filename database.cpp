#include "database.h"

void Entry::printSelf() { cout << key << " " << name << " " << email << endl; }

uint32_t fetchNumEntries(int fd) {
  // need to find how many entries are in the table on startup (from end of the
  // DB file)
  uint32_t intValue;
  ssize_t bytesRead = pread(fd, &intValue, PAGE_SIZE, MAX_PAGES * PAGE_SIZE);
  if (bytesRead == -1)
    exit(EXIT_FAILURE);
  if (bytesRead != sizeof(intValue)) return 0;
  return intValue;
}

Table::Table() {
  this->numEntries = 0;
  for (int i = 0; i < MAX_PAGES; i++) {
    this->pages[i] = nullptr;
  }
  this->fd = open(TABLE_FILENAME.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (this->fd == -1) {
    cout << "Error opening file" << endl;
    exit(EXIT_FAILURE);
  }
  this->numEntries = fetchNumEntries(this->fd);
}

void Database::prompt() { cout << "DB > "; }

int Database::metaCommand(string &input, Table& table) {
  if (input == ".exit") {
    table.closeTable();
    exit(EXIT_SUCCESS);
  }
  return 1;
}

int Database::parseCommand(string &input, Statement &st) {
  if (input.substr(0, 6) == "insert") {
    st.type = StatementType::INSERT;
    stringstream temp(input);
    string trash;
    temp >> trash;
    if (!(temp >> st.new_entry.key >> st.new_entry.name >> st.new_entry.email))
      return 1;
  } else if (input == "select") {
    st.type = StatementType::SELECT;
  } else if (input.substr(0, 6) == "delete") {
    st.type = StatementType::DELETE;
  } else {
    return 1;
  }
  return 0;
}

int Database::executeStatement(Statement st, Table &table) {
  switch (st.type) {
  case (StatementType::SELECT):
    Entry e;
    for (uint32_t i = 0; i < table.numEntries; i++) {
      Database::deserializeEntry(Database::entrySlot(table, i), e);
      e.printSelf();
    }
    break;
  case (StatementType::INSERT):
    if (table.numEntries == MAX_ENTRIES) {
      cout << "TABLE IS FULL. DID NOT ADD ENTRY." << endl;
    } else {
      Database::serializeEntry(st.new_entry,
                               Database::entrySlot(table, table.numEntries));
      table.numEntries += 1;
    }
    break;
  default:
    return 1;
  }
  return 0;
}

void *Database::entrySlot(Table &table, uint32_t entry_num) {
  uint32_t page_num = entry_num / ENTRIES_PER_PAGE;
  void *page = table.getPage(page_num);
  uint32_t entry_offset = entry_num % ENTRIES_PER_PAGE;
  uint32_t byte_offset = entry_offset * ENTRY_SIZE;
  return static_cast<char *>(page) + byte_offset;
}

void Database::deserializeEntry(void *source, Entry &destination) {
  memcpy(&(destination.key), static_cast<char *>(source) + KEY_OFFSET,
         KEY_SIZE);
  memcpy(&(destination.name), static_cast<char *>(source) + NAME_OFFSET,
         NAME_SIZE);
  memcpy(&(destination.email), static_cast<char *>(source) + EMAIL_OFFSET,
         EMAIL_SIZE);
}

void Database::serializeEntry(Entry &source, void *destination) {
  memcpy(static_cast<char *>(destination) + KEY_OFFSET, &(source.key),
         KEY_SIZE);
  memcpy(static_cast<char *>(destination) + NAME_OFFSET, &(source.name),
         NAME_SIZE);
  memcpy(static_cast<char *>(destination) + EMAIL_OFFSET, &(source.email),
         EMAIL_SIZE);
}

void print_page(void *page, size_t size) {
  // Cast the void pointer to a char pointer to allow byte-wise access
  unsigned char *p = (unsigned char *)page;
  // Print each byte in hexadecimal format
  for (size_t i = 0; i < size; ++i) {
    printf("%02X ", p[i]); // Print byte in hexadecimal format
    if ((i + 1) % 16 == 0) // Print a newline every 16 bytes
      cout << endl;
  }
  cout << endl;
}

void *Table::getPage(uint32_t pageNum) {
  void *page = this->pages[pageNum];
  if (page == nullptr) {
    page = this->pages[pageNum] = new char[PAGE_SIZE];
    ssize_t bytesRead = pread(this->fd, page, PAGE_SIZE, PAGE_SIZE * pageNum);
    if (bytesRead == -1) {
      cout << "Error loading table" << endl;
      exit(EXIT_FAILURE);
    }
  }
  return page;
}