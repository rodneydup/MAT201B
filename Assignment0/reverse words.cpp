#include <iostream>
#include <string>

using namespace std;

int main()
{
    while (true)
    {
        printf("Type a sentence then hit RETURN: ");
        string line;
        getline(cin, line);
        if (!cin.good())
        {
            printf("Done!\n");
            return 0;
        }

        string word;
        
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == ' ') {
                for (auto reviterate = word.crbegin(); reviterate != word.crend(); reviterate++) cout << *reviterate;
                cout << ' ';
                word.clear();
                continue;
            }
            // add character to word
            word.push_back(line[i]);
            
            if (i == line.length() - 1) for (auto reviterate = word.crbegin(); reviterate != word.crend(); reviterate++) cout << *reviterate;
        }
        cout << "\n";
        return 0;
    }
}