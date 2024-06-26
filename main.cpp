#include "database.h"

int main() {
  Table t;
  Database d;
  while (1) {
    d.prompt();
    string input;
    getline(cin, input);
    if (input[0] == '.') {
      if (!d.metaCommand(input, t)) {
        continue;
      } else
        cout << "Meta command not recognized" << endl;
    } else {
      Statement st;
      if (d.parseCommand(input, st)) {
        cout << "Syntax error: " << input << endl;
        continue;
      }
      d.executeStatement(st, t);
    }
  }
}