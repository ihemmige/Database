#include "database.h"

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

void Table::flushPage(uint32_t pageNum) {
  void *page = this->pages[pageNum];
  if (page == nullptr) {
    return;
  }
  writePage(this->fd, page, pageNum);
  storeNumEntries(this->numEntries, this->fd);
}

void Table::closeTable() {
    for (int i = 0; i < MAX_PAGES; i++) {
      if (this->pages[i]) {
          this->flushPage(i);
          delete[] reinterpret_cast<char*>(this->pages[i]);
      }    
    }
    close(this->fd);
}