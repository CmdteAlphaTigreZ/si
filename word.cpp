#include "word.h"
#include <fstream>
#include <iostream>
using namespace std;

Word::Word()
{
        filetype = "";
}

void Word::set_type(string type)
{
        filetype = type;
}

string Word::get_type()
{
        return filetype;
}

void Word::set_dict(string direction)
{
        // Initialize dictionary with all the translations
        string file;

        if ((filetype == "cpp") || (filetype == "c"))
                file = "dict_cpp.txt";
        else if (filetype == "py")
                file = "dict_py.txt";
        else {
                cerr << "ERROR: improper or no filetype specified" << endl;
                exit(EXIT_FAILURE);
        }

        ifstream infile(file);
        string eng, esp;
        while(infile >> esp >> eng)
        {
                if (direction == "si")
                        dictionary.emplace(esp, eng);
                else
                        dictionary.emplace(eng, esp);
        }
}

void Word::set_out(string new_out)
{
        out.open(new_out);
}

void Word::set_in(string new_in)
{
        in.open(new_in);
}

void Word::close_files()
{
        in.close();
        out.close();
}

/* analyzes and writes any non-alphanumeric characters at the front of the word
 * and sets any ignore flags if necessary */
void Word::check_front()
{
        string com_check;
        com_check.clear();

        while (!isalnum(word.front()) && (word.size() >= 1)) {
                com_check += word.front();
                out << word.front();
                word.erase(0,1);
                // ignores for Python
                if (filetype == "py") {
                        if (com_check == "#") {
                                py_ignore = true;
                        }
                }
                // ignores for C and C++
                else {
                        if (com_check == "//") {
                                ignore = true;
                        }
                        else if (com_check == "/*") {
                                long_ignore = true;
                        }
                        else if (com_check == "*/" && long_ignore) {
                                long_ignore = false;
                        } 
                        else if (com_check == "\x22" && !q_ignore) {
                                q_ignore = true;
                        }
                        else if(com_check == "\x22" && q_ignore) {
                                q_ignore = false;
                        }
                }
        }
}

/* loops through the whole word taken in. if symbols are found,
 * splits word up for piece-by-piece analysis and stores
 * symbols in a vector, that way a theoretically valid "word"
 * like ca<<rzcb((mi_matríz->mi_palabra.tamaño))<<terml;
 * gets everything necessary to change to English changed.
 */
void Word::split_word()
{
        unsigned long i = 0;
        while (i != word.length()) {
                /* loop through word until non-alphanumeric is found */
                if(!isalnum(word[i]) && (!isspace(word[i])) 
                   && word[i] != '_') { 
                        /* put word up to that point in vector */
                        wvec.push_back(word.substr(0,i));
                        word.erase(0,i);
                        i = 0;
                        /* parse through non-alphanums and set ignores */
                        while (!isalnum(word[i]) && !isspace(word[i]) &&
                                i != word.length()) {
                                i++;
                        }
                        /* put non-alphanums in vector */
                        symvec.push_back(word.substr(0,i));
                        word.erase(0,i);
                        i = 0;
                        continue;
                }
                i++;
        }
        if (word.length() != 0) // i.e. no symbols found
                wvec.push_back(word);
}

/* empties word vector one word at a time, translating them and then writing
 * their translations, or lack thereof. Then prints the symbols following 
 * the words, if any exist, and removes any ignore flags that have been set
 * if a symbol indicates they should be turned off. */
void Word::print_word()
{
        string sym;

        while (!wvec.empty()) {
                word = wvec.front();
                wvec.erase(wvec.begin());

                if (long_ignore || q_ignore || ignore || py_ignore) {
                        out << word;
                }
                else 
                        out << translate_word(word);

                if (!symvec.empty()) {
                        sym = symvec.front();
                        out << sym;
                        /* turn on and off ignores for C and C++ */
                        if (filetype != "py") {
                                if ((sym.find("\x22") != string::npos) 
                                     && !q_ignore)
                                        q_ignore = true;
                                else if ((sym.find("/*") != string::npos) 
                                          && !long_ignore)
                                        long_ignore = true;
                                else if ((sym.find("//") != string::npos)
                                          && !ignore)
                                        ignore = true;
                                else if ((sym.find("*/") != string::npos)
                                     && long_ignore)
                                        long_ignore = false;
                                else if ((sym.find("\x22") != string::npos)
                                          && q_ignore)
                                        q_ignore = false;
                        }
                        symvec.erase(symvec.begin());
                }
        }

        /* print whitespace where necessary */
        while(isspace(in.peek())){
                while(in.peek() == ' '){
                        out << ' ';
                        in.get();
                }
                while(in.peek() == '\t'){
                        out << "\t";
                        in.get();
                }
                while(in.peek() == '\n'){
                        out << endl;
                        in.get();
                        if ((ignore) && (filetype != "py"))
                                ignore = false;
                        else if ((py_ignore) && (filetype == "py"))
                                py_ignore = false;
                }
        }
}

/* runs translations with map in dict.txt */
string Word::translate_word(string word)
{
        if (dictionary[word] != "") {
                //translation exists
                return dictionary[word];
        } else {
                return word;
        }
}

/* here's your loop, all nice and compressed! */
void Word::write()
{       
        while (in >> word) {
                check_front();
                split_word();
                print_word();
        }
}
