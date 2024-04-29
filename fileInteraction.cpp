#include "database.h"

uint32_t fetchNumEntries(int fd) {
  // need to find how many entries are in the table on startup (from end of the
  // DB file)
  uint32_t intValue;
  ssize_t bytesRead = pread(fd, &intValue, PAGE_SIZE, MAX_PAGES * PAGE_SIZE);
  if (bytesRead == -1 || bytesRead != sizeof(intValue))
    exit(EXIT_FAILURE);
  return intValue;
}

void storeNumEntries(uint32_t numEntries, int fd) {
  // the number of entries is stored at the end of the DB file, after all pages
  ssize_t bytesWritten =
      pwrite(fd, &numEntries, sizeof(numEntries), MAX_PAGES * PAGE_SIZE);
  if (bytesWritten == -1 || bytesWritten != sizeof(numEntries)) {
    cout << "Failed to write through to DB." << endl;
    exit(EXIT_FAILURE);
  }
}

void writePage(int fd, void *page, uint32_t pageNum) {
  ssize_t bytesWritten = pwrite(fd, page, PAGE_SIZE, pageNum * PAGE_SIZE);
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

void Table::flushPage(uint32_t pageNum) {
  void *page = this->pages[pageNum];
  if (page == nullptr) {
    return;
  }
  writePage(this->fd, page, pageNum);
  storeNumEntries(this->numEntries, this->fd);
}

void Table::loadTable() {
  void *buf = malloc(PAGE_SIZE);
  this->numEntries = fetchNumEntries(this->fd);
  for (int i = 0; i < MAX_PAGES; i++) {
    ssize_t bytesRead = pread(this->fd, buf, PAGE_SIZE, PAGE_SIZE * i);
    if (bytesRead == -1) {
      cout << "Error loading table" << endl;
      exit(EXIT_FAILURE);
    } else if (bytesRead > 0) {
      void *pagePtr = this->getPagePointer(i);
      memcpy(pagePtr, buf, PAGE_SIZE);
    }
  }
  free(buf);
}