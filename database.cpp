#include "database.h"
#include <unistd.h>
#include <fcntl.h>

void Entry::printSelf() { cout << key << " " << name << " " << email << endl; }

Table::Table() { 
  numEntries = 0; 
  for (int i = 0; i < MAX_PAGES; i++) {
    this->pages[i] = nullptr;
  }
  this->fd = open("data.bin", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (this->fd == -1) {
      cout << "Error opening file" << endl;
      exit(EXIT_FAILURE);
    }
}

Table::~Table() {
  for (int i = 0; i < MAX_PAGES; i++) {
    free(pages[i]);
  }
  // char buf[sizeof(this->numEntries)];
  // memcpy(buf, &(this->numEntries), sizeof(this->numEntries));
  // ssize_t bytesWritten = pwrite(this->fd, buf, sizeof(buf), 0);
  // if (bytesWritten == -1 || bytesWritten != sizeof(buf)) {
  //   cout << "Failed to write through to DB." << endl;
  // }
  close(this->fd);
}

void Database::prompt() { cout << "DB > "; }

int Database::metaCommand(string &input) {
  if (input == ".exit") {
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
    table.flushPage(table.numEntries/ENTRIES_PER_PAGE);
    break;
  default:
    return 1;
  }
  return 0;
}

void *Database::entrySlot(Table &table, uint32_t entry_num) {
  uint32_t page_num = entry_num / ENTRIES_PER_PAGE;
  void *page = table.pages[page_num];
  if (page == nullptr) {
    page = table.pages[page_num] = malloc(PAGE_SIZE);
  }
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

void Table::flushPage(uint32_t page_num) {
  void *page = this->pages[page_num];
  if (page == nullptr) {
    return;
  }
  ssize_t bytesWritten = pwrite(this->fd, page, PAGE_SIZE, page_num * PAGE_SIZE);
  if (bytesWritten == -1 || bytesWritten != PAGE_SIZE) {
    cout << "Failed to write through to DB." << endl;
    exit(EXIT_FAILURE);
  }
}

void* Table::getPagePointer(uint32_t page_num) {
  if (this->pages[page_num]) return this->pages[page_num];
  void* pagePtr = this->pages[page_num] = malloc(PAGE_SIZE);
  return pagePtr;
}

void Table::readPageOne() {
  void* buf = malloc(PAGE_SIZE);
  int fd = open("data.bin", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  ssize_t bytesRead = pread(fd, buf, PAGE_SIZE, 0);
  uint32_t intValue;
  memcpy(&intValue, buf, sizeof(intValue));
  this->numEntries = intValue;
  cout << intValue << endl;
  close(fd);
}

void Table::loadTable() {
  this->fd = open("data.bin", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  void* buf = malloc(PAGE_SIZE);

  // this->readPageOne();

  for (int i = 0; i < MAX_PAGES; i++) {
    ssize_t bytesRead = pread(this->fd, buf, PAGE_SIZE, PAGE_SIZE * i);
    if (bytesRead == -1) {
      cout << "Error loading table" << endl;
      exit(EXIT_FAILURE);
    } else if (bytesRead == 0) continue;
    else {
        void* pagePtr = this->getPagePointer(i);
        memcpy(pagePtr, buf, PAGE_SIZE);
    }
  }
  free(buf);
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