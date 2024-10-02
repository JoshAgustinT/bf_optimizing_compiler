/*
Joshua Tlatelpa-Agustin
9/18/24
bf language compiler
written for adv compilers course
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <sstream>
#include <cassert>
using namespace std;

vector<char> program_file;
ofstream *output_file;
int loop_num = -1;
int seek_loop = -1;
stack<int> myStack;
int tape_size = 1048576;

bool simple_loop_flag = false;
bool seek_flag = false;
bool optimization_flag = false;
string bf_file_name = "";

/*
jasm, short for josh asm :] outputs to output_file.
*/
void jasm(string text)
{
    *output_file << text << endl;
}

void print_padding()
{
    *output_file << endl;
}

/*
naive implementation, also easier to debug
*/
void bf_assembler(char token)
{

    switch (token)
    {
    case '>':
        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // add one to pointer address
        jasm("addq    $1, %rax");
        // Store the adjusted pointer back at -8(%rbp)
        jasm("movq    %rax, -8(%rbp)");
        // tape_position++;
        break;
    case '<':

        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // remove one from pointer address
        jasm("subq    $1, %rax");
        // Store the adjusted pointer back at -8(%rbp)
        jasm("movq    %rax, -8(%rbp)");
        // tape_position--;
        break;
    case '+':

        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // Add 1 to the byte
        jasm("addb    $1, %cl");

        // Store the modified byte back to the address in %rax
        jasm("movb    %cl, (%rax)");

        // tape[tape_position] += 1;
        break;
    case '-':

        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // Decrrement byte in %cl
        jasm("subb    $1, %cl");

        // Store the modified byte back to the address in %rax
        jasm("movb    %cl, (%rax)");

        // tape[tape_position] -= 1;
        break;
    case '.':

        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load the byte from the address into %al (to use with putc)
        jasm("movb    (%rax), %al");

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        jasm("movq    stdout(%rip), %rsi");
        // Move and sign-extend byte in %al to %edi
        jasm("movsbl  %al, %edi");

        // Call putc to print the character
        jasm("call    putc@PLT");

        // cout << tape[tape_position];
        break;
    case ',':

        // Move the file pointer for stdin into the %rdi register
        jasm("movq    stdin(%rip), %rdi");

        // Call the getc function to read a character from stdin (returned in %al)
        jasm("call    getc@PLT");
        // Move the byte from %al into %bl
        jasm("movb    %al, %bl");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Store the byte from %bl into the memory pointed to by %rax
        jasm("movb    %bl, (%rax)");

        // char nextByte;
        // cin.get(nextByte);
        // tape[tape_position] = nextByte;
        break;
    case '[':
    {
        loop_num++;
        myStack.push(loop_num);

        string start_label = "start_loop_";
        start_label += to_string(loop_num);
        string end_label = "end_loop_";
        end_label += to_string(loop_num);

        jasm(start_label + ":");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");
        // jump to matching end label if 0
        jasm("cmpb    $0, %cl");
        jasm("je      " + end_label);
    }

    break;
    case ']':
    {
        int match_loop = myStack.top();
        myStack.pop();
        string start_label = "start_loop_";
        start_label += to_string(match_loop);

        string end_label = "end_loop_";
        end_label += to_string(match_loop);

        jasm(end_label + ":");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // jump to matching start label if not 0
        jasm("cmpb    $0, %cl");
        jasm("jne      " + start_label);
    }

    break;
    default:
        // non bf instruction, so we ignore
        break;
    } // end switch
}

bool startsWith(const std::string &str, const std::string &prefix)
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

void asm_setup()
{
    // Assembly setup
    jasm(".file	\"bf compiler\"");

    jasm(".section .data");

    if (optimization_flag || seek_flag)
    {
        // offset masks= 1,2,4,8,16
        jasm(".p2align 5");
        jasm(".four_offset_mask:");
        jasm(".quad   282578783371521");
        jasm(".quad   282578783371521");
        jasm(".quad   282578783371521");
        jasm(".quad  282578783371521");

        jasm(".p2align 5");
        jasm(".one_offset_mask:");
        jasm(".quad   0");
        jasm(".quad   0");
        jasm(".quad   0");
        jasm(".quad   0");

        jasm(".p2align 5");
        jasm(".two_offset_mask:");
        jasm(".quad   281479271743489");
        jasm(".quad   281479271743489");
        jasm(".quad   281479271743489");
        jasm(".quad  281479271743489");

        jasm(".p2align 5");
        jasm(".eight_offset_mask:");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");

        jasm(".p2align 5");
        jasm(".sixteen_offset_mask:");
        jasm(".quad   72340172838076673");
        jasm(".quad   282578800148737");
        jasm(".quad   72340172838076673");
        jasm(".quad  282578800148737");

        jasm(".p2align 5");
        jasm(".thirty_two_offset_mask:");
        jasm(".quad   72340172838076673");
        jasm(".quad   72340172838076673");
        jasm(".quad   72340172838076673");
        jasm(".quad  282578800148737");
    }

    jasm(".text");
    jasm(".section	.text");
    jasm(".globl	main");
    jasm(".type	main, @function");
    print_padding();

    jasm("main:");

    jasm("pushq	%rbp");
    jasm("movq	%rsp, %rbp");
    // Allocate 16 bytes of stack space for local variables
    jasm("subq	$16, %rsp");
    // Allocate 100,000 bytes with malloc
    jasm("movl	$" + to_string(tape_size) + ", %edi");

    jasm("call	malloc@PLT");
    // Store the pointer returned by malloc in the local variable at -8(%rbp)
    jasm("movq	%rax, -8(%rbp)");

    // Calculate the address 50,000 bytes into the allocated memory
    jasm("addq    $" + to_string(tape_size / 2) + ", %rax"); // Add the offset to %rax

    // Store the adjusted pointer back at -8(%rbp)
    jasm("movq    %rax, -8(%rbp)");
    jasm("movq    -8(%rbp), %r13"); // keep our copy of the cell address at r13
}

void asm_cleanup()
{
    // Set the return value to 0 (successful completion)
    jasm("movl    $0, %eax");
    // Proper stack cleanup
    jasm("movq    %rbp, %rsp");
    // Restore the old base pointer
    jasm("popq    %rbp");
    // Return from the function
    jasm("ret");
    print_padding();
}
/*
Turn  vector<char> bf program to vector<string>, to support saving complex instructions
also this list will not contain non-instruction
*/
vector<string> init_optimized_program_list(vector<char> list)
{
    vector<string> return_list;

    for (int i = 0; i < list.size(); i++)
    {
        char token = list[i];
        switch (token)
        {
        case '>':
            return_list.push_back(string(1, token));
            break;
        case '<':
            return_list.push_back(string(1, token));
            break;
        case '+':
            return_list.push_back(string(1, token));
            break;
        case '-':
            return_list.push_back(string(1, token));
            break;
        case '.':
            return_list.push_back(string(1, token));
            break;
        case ',':
            return_list.push_back(string(1, token));
            break;
        case '[':
            return_list.push_back(string(1, token));
            break;
        case ']':
            return_list.push_back(string(1, token));
            break;
        default:
            // non bf instruction, so we ignore
            break;
        } // end switch

    } // end for loop

    return return_list;
}

/*
returns an unordered_set<int> of all starting loop indices.
*/
unordered_set<int> get_loop_indices(vector<string> list)
{
    unordered_set<int> return_set;

    for (int i = 0; i < list.size(); i++)
    {
        string token = list[i];

        if (token == "[")
            return_set.insert(i);
    }
    return return_set;
}

/*
Returns associated loop of given index from passed in list.
ie, [*.>><], given program index of [, will return [*.>><]
*/
vector<string> get_loop_string(int j, vector<string> list)
{
    vector<string> return_list;
    if (list[j] == "[")
    {
        return_list.push_back("[");
        int count = 1;
        while (count != 0)
        {
            j++;

            if (list[j] == "[")
                count++;

            if (list[j] == "]")
                count--;

            // Code for handling valid instr
            return_list.push_back(list[j]);
        }

        return return_list;
    }

    cout << "couldn't find '[' in get_loop_string(), wrong index?? " << endl;
    return return_list;
}

// tells us net change of our initial cell
int cell_net_change(vector<string> loop_string)
{
    int cell = 0;
    int cell_value = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token == "<")
            cell--;
        if (token == ">")
            cell++;

        if (token == "+")
        {
            if (cell == 0)
            {
                cell_value++;
            }
        }
        if (token == "-")
        {
            if (cell == 0)
            {
                cell_value--;
            }
        }

    } // end for loop

    return cell_value;
}

/*
Checks if input string is a simple loop
*/
bool is_simple_loop(vector<string> loop_string)
{
    bool answer = true;
    int cell_offset = 0;
    int loop_cell_value = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token == ",")
        {
            answer = false;
            break;
        }
        if (token == ".")
        {
            answer = false;
            break;
        }
        if (token == "[")
        {
            answer = false;
            break;
        }
        if (token == "]")
        {
            answer = false;
            break;
        }
        if (token == "<")
            cell_offset--;
        if (token == ">")
            cell_offset++;

        if (token == "+")
        {
            if (cell_offset == 0)
            {
                loop_cell_value++;
            }
        }
        if (token == "-")
        {
            if (cell_offset == 0)
            {
                loop_cell_value--;
            }
        }
        if (token.compare(0, 12, "expr_simple:") == 0)
        {
            answer = false;
            break;
        }
        if (token.compare(0, 10, "expr_seek:") == 0)
        {
            answer = false;
            break;
        }

    } // end for loop

    if (cell_offset != 0)
        answer = false;

    if (abs(loop_cell_value) != 1)
        answer = false;

    return answer;
}

bool is_power_of_two(int n)
{
    bool answer = false;
    switch (n)
    {
    case -32:
    case -16:
    case -8:
    case -4:
    case -2:
    case 2:
    case 4:
    case 16:
    case 32:
        answer = true;
        break;
    default:
        break;
    }

    return answer;
}
/*
Checks if input loop is a power of 2 seek loop
*/
int is_power_two_seek_loop(vector<string> loop_string)
{
    int offset = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token != ">" && token != "<")
        {
            return false;
        }
        if (token == ">")
            offset++;

        if (token == "<")
            offset--;

    } // end for loop

    if (!is_power_of_two(offset))
        return false;
    // basically only true if we only have one type of seek
    return offset;
}

/*
Checks if input loop is a seek loop
*/
int is_seek_loop(vector<string> loop_string)
{
    int offset = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token != ">" && token != "<")
        {
            return false;
        }
        if (token == ">")
            offset++;

        if (token == "<")
            offset--;

    } // end for loop

    // basically only true if we only have one type of seek
    return offset;
}

/*
Prints all elements in a vector<string> with no new line in between elements, with a new line at end.
*/
void print_string_vector(vector<string> list)
{
    for (auto token : list)
        cout << token;

    cout << endl;
}
string expr_dict_to_string(map<int, int> dict, int loop_increment)
{

    string sb = "expr_simple:";

    // will tell us if we have +1 or -1 simple loops.
    if (loop_increment >= 0)
        sb += "+";

    else
        sb += "-";

    for (const auto &pair : dict)
    {
        sb += to_string(pair.first);
        sb += ":";
        sb += to_string(pair.second);
        sb += ",";
    }

    return sb;
}

void print_int_int_map(map<int, int> dict)
{

    for (const auto &pair : dict)
    {
        cout << "{";
        cout << to_string(pair.first);
        cout << ":";
        cout << to_string(pair.second);
        cout << "}";
        cout << endl;
    }
}
vector<string> optimize_simple_loop(int loop_index, int loop_increment, vector<string> loop, vector<string> program)
{

    map<int, int> dict;
    int tape_offset = 0;

    for (auto token : loop)
    {
        if (token == ">")
            tape_offset++;
        if (token == "<")
            tape_offset--;
        if (token == "+")
        {
            if (dict.find(tape_offset) != dict.end())
                dict[tape_offset] = dict[tape_offset] + 1;
            else
                dict[tape_offset] = 0 + 1;
        }
        if (token == "-")
        {
            if (dict.find(tape_offset) != dict.end())
                dict[tape_offset] = dict[tape_offset] - 1;
            else
                dict[tape_offset] = 0 - 1;
        }
    } // end loop

    for (int i = 0; i < loop.size(); i++)
    {
        program[loop_index + i] = " ";
    }
    // remove loop if [], should never fire unless infinite loop
    // if (loop.size() == 2)
    //     return program;

    program[loop_index] = expr_dict_to_string(dict, loop_increment);

    return program;
}

map<int, int> expr_string_to_dict(string expr)
{
    map<int, int> dict;

    // Check if the string starts with "expr_simple:"
    if (expr.compare(0, 12, "expr_simple:") == 0)
    {
        // Remove the prefix, plus the sign attached
        expr.erase(0, 13);
        // cout << expr );
        //  example- 0:-1,1:7,2:10,3:3,4:1,
        std::string first, second;

        std::istringstream ss(expr); // Use stringstream for easy tokenization

        while (std::getline(ss, first, ':'))
        {
            // Extract string before ':'
            std::getline(ss, second, ','); // Extract string after ':' but before ','

            int int_first = stoi(first);
            int int_second = stoi(second);
            if (dict.find(int_first) != dict.end())
            {
                cout << "why was there a duplicate while parsing expr into dict?" << endl;
                cout << int_first << "," << int_second << endl;
                assert(1 == 0);
            }
            else
                dict[int_first] = int_second;
        }

    } // end expr_simple
    else
    {
        cout << "expr_string_to_dict(), you didn't pass in an expr string?" << endl;
        assert(1 == 0);
    }

    return dict;
}

int get_expr_seek_offset(string expr)
{
    int offset = 0;
    if (expr.compare(0, 10, "expr_seek:") == 0)
    {
        // Remove the prefix, plus the sign attached
        expr.erase(0, 10);
        offset = stoi(expr);
        // cout<< offset<<endl;
    }
    else
    {
        cout << "erm u didnt pass in a expr_seek string... in get_expr_seek_offset()" << endl;
        assert(1 == 0);
    }

    return offset;
}

/*
our optimized assembler, usees vector strings to store instructions which allow for easier modification of bf source
we also further optimize by keeping out tape location at register r13
*/
void bf_string_assembler(string token)
{
    // print_padding();

    if (token == ">")
    {

        // add one to pointer address
        jasm("addq    $1, %r13");
    }
    if (token == "<")
    {

        // remove one from pointer address
        jasm("addq    $-1, %r13");
    }
    if (token == "+")
    {

        // Load current cell byte into %cl (lower 8 bits)
        jasm("movb    (%r13), %cl");

        // Add 1 to the byte
        jasm("addb    $1, %cl");

        // Store the modified byte back to the address in %r13, our current cell
        jasm("movb    %cl, (%r13)");
    }
    if (token == "-")
    {

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%r13), %cl");

        // Decrrement byte in %cl
        jasm("subb    $1, %cl");

        // Store the modified byte back to the address in %r13, our current cell
        jasm("movb    %cl, (%r13)");
    }
    if (token == ".")
    {

        // Load the byte from the address into %al (to use with putc)
        jasm("movb    (%r13), %al");

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        jasm("movq    stdout(%rip), %rsi");
        // Move and sign-extend byte in %al to %edi
        jasm("movsbl  %al, %edi");

        // Call putc to print the character
        jasm("call    putc@PLT");

        // cout << tape[tape_position];
    }
    if (token == ",")
    {

        // Move the file pointer for stdin into the %rdi register
        jasm("movq    stdin(%rip), %rdi");

        // Call the getc function to read a character from stdin (returned in %al)
        jasm("call    getc@PLT");
        // Move the byte from %al into %bl
        jasm("movb    %al, %bl");

        // Store the byte from %bl into r13 our current cell
        jasm("movb    %bl, (%r13)");

        // char nextByte;
        // cin.get(nextByte);
        // tape[tape_position] = nextByte;
    }
    if (token == "[")
    {

        loop_num++;
        myStack.push(loop_num);

        string start_label = "start_loop_" + to_string(loop_num);

        string end_label = "end_loop_" + to_string(loop_num);

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%r13), %cl");
        // jump to matching end label if 0
        jasm("cmpb    $0, %cl");
        jasm("je      " + end_label);
        jasm(start_label + ":");
    }
    if (token == "]")
    {
        int match_loop = myStack.top();
        myStack.pop();
        string start_label = "start_loop_" + to_string(match_loop);
        string end_label = "end_loop_" + to_string(match_loop);
        // Load the pointer from -8(%rbp) into %rax
        // jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%r13), %cl");

        // jump to matching start label if not 0
        jasm("cmpb    $0, %cl");
        jasm("jne      " + start_label);
        jasm(end_label + ":");
    }

    if (startsWith(token, "expr_simple:"))
    {
        // jasm("opt:");
        string sign_of_loop;

        // if +, we perform loop 256-255 times
        // if -, we perform full p[0] times
        if (startsWith(token, "expr_simple:+"))
            sign_of_loop = "+";

        if (startsWith(token, "expr_simple:-"))
            sign_of_loop = "-";

        map<int, int> simple_expr = expr_string_to_dict(token);

        // 8 bit regists, AH AL BH BL CH CL DH DL
        // ch and bl work

        // save current cell contents at start of loop in rcx

        if (sign_of_loop == "-")
            jasm("movq    (%r13), %rcx");

        if (sign_of_loop == "+")
        {
            // input can't be more than 255 so we're good
            jasm("movq    $256, %rcx ");
            jasm("subq    (%r13), %rcx  ");
        }

        for (const auto &pair : simple_expr)
        {
            if (pair.first == 0)
                continue;

            // pair.first = pointer offset
            // pair.second = cell +- change per loop

            // copy address of the first loop cell
            jasm("movq   %r13, %r12");
            // adjust pointer by our offset
            jasm("addq   $" + to_string(pair.first) + ", %r12");
            // set our constant change of cell
            jasm("movq    $" + to_string((pair.second)) + ", %r15");
            // constant multiplied by times loop happens
            jasm("imul   %rcx, %r15");

            jasm("addb    %r15b , (%r12)");
        }
        // our loop should always end in 0, this assures it, but would break
        // intentional infinite loops ¯\_(ツ)_/¯, saves us like 5 instr per loop
        jasm("movb    $0, (%r13)");

        print_padding();
    }

    if (startsWith(token, "expr_seek:"))
    {
        print_padding();

        int seek_offset = get_expr_seek_offset(token);

        if (is_power_of_two(seek_offset))
        {
            //cout<< seek_offset<<endl;

            seek_loop++;
            string start_label = "start_seek_loop_" + to_string(seek_loop);
            string end_label = "end_seek_loop_" + to_string(seek_loop);

            // jasm("opt:");

            jasm("movb    (%r13), %cl");
            jasm("cmpb    $0, %cl");

            jasm("je      " + end_label);

            // this makes our offset masks easier to reason about (i like seeing 32)
            // subtracting 32, so our loop can always add 32, and on first iterations will be 0.

            if (seek_offset > 0)
            {
                jasm("addq $1, %r13");
                jasm("subq    $32, %r13");
            }
            else
            {
                //same alignment reason, but we don't need to offset our first loop since we want to read the previous bytes 
                jasm("addq $1, %r13");
                // jasm("addq    $32, %r13");
            }

            // Loop for checking bytes in chunks of 32
            ////////////////////////////////////////////////////////////
            jasm(start_label + ":");

            if (seek_offset > 0)
                jasm("addq    $32, %r13");
            else
                jasm("subq    $32, %r13"); // CHANGE IF neg i think

            if (abs(seek_offset) == 4)
                jasm("vmovdqa .four_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 1)
                jasm("vmovdqa .one_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 2)
                jasm("vmovdqa .two_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 8)
                jasm("vmovdqa .eight_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 16)
                jasm("vmovdqa .sixteen_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 32)
                jasm("vmovdqa .thirty_two_offset_mask(%rip), %ymm0");

            jasm("vpor    (%r13), %ymm0, %ymm0");
            jasm("vpxor   %xmm1, %xmm1, %xmm1");
            jasm("vpcmpeqb        %ymm1, %ymm0, %ymm0");
            jasm("vpmovmskb       %ymm0, %eax");
            jasm("testl   %eax, %eax");
            jasm("je      " + start_label);

            if (seek_offset > 0)
            {
                jasm("bsfl    %eax, %eax"); // CHANGE IF neg i think
                jasm("addq %rax, %r13");
            }

            else
            {
                jasm("bsrl    %eax, %eax");
                jasm("addq %rax, %r13");

            }

            /////////////////////////////////////////////////////////////////
            // save offset

            jasm(end_label + ":");
            print_padding();
        }

        // all other seek loops
        else
        {
            seek_loop++;
            string start_label = "start_seek_loop_" + to_string(seek_loop);
            string end_label = "end_seek_loop_" + to_string(seek_loop);

            // jasm("opt:");
            //  Load byte into %cl (lower 8 bits)
            jasm("movb    (%r13), %cl");
            // jump to matching end label if 0
            jasm("cmpb    $0, %cl");
            jasm("je      " + end_label);

            jasm(start_label + ":");

            // remove one from pointer address
            jasm("addq    $" + to_string(seek_offset) + ", %r13");

            // Load byte into %cl (lower 8 bits)
            jasm("movb    (%r13), %cl");
            // jump to matching end label if 0
            jasm("cmpb    $0, %cl");
            jasm("jne      " + start_label);

            jasm(end_label + ":");
        }
    } // end seek
} // end asm_string

vector<string> optimize_seek_loop(int loop_index, int seek_offset, vector<string> loop, vector<string> program)
{
    string sb = "expr_seek:";
    for (int i = 0; i < loop.size(); i++)
    {
        program[loop_index + i] = " ";
    }
    sb += to_string(seek_offset);

    program[loop_index] = sb;
    return program;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        cout << "No input file?" << endl;
        return 1;
    }
    for (int i = 0; i < argc; i++)
    {
        string args = argv[i];

        if (args == "-O")
            simple_loop_flag = true;
        if (args == "-v")
            seek_flag = true;
        if (args == "-O1")
            optimization_flag = true;
        if (args.find(".b") != std::string::npos)
        {
            assert(bf_file_name == "");

            bf_file_name = args;
        }
    }

    // Open bf file
    ifstream inputFile(bf_file_name);
    if (!inputFile)
    {
        cout << "Couldn't open file: " << bf_file_name << endl;
        return 1;
    }
    // Read the file into a vector of chars
    program_file.assign((istreambuf_iterator<char>(inputFile)),
                        istreambuf_iterator<char>());
    // Close the file
    inputFile.close();

    // create our output file
    ofstream outFile("bf.s");
    output_file = &outFile;

    if (!outFile)
    {
        cout << "could not create output file" << endl;
        return 2; // Return with an error code
    }
    /*
    DONT TOUCH register r13! it's where we keep our current cell address
    */
    asm_setup();

    if (simple_loop_flag || seek_flag || optimization_flag)
    {
        vector<string> optimized_program = init_optimized_program_list(program_file);

        // print program without non-instructions
        // print_string_vector(optimized_program);
        unordered_set<int> loop_indices = get_loop_indices(optimized_program);

        for (auto token : loop_indices)
        {
            vector<string> loop = get_loop_string(token, optimized_program);

            if (is_simple_loop(loop))
            {
                int loop_increment = cell_net_change(loop);

                if (abs(loop_increment) != 1)
                {

                    cout << "huh??? simple loop should only +- 1 each iteration!" << endl;
                    print_string_vector(loop);
                    assert(1 == 0);
                }

                if (simple_loop_flag || optimization_flag)
                    optimized_program = optimize_simple_loop(token, loop_increment, loop, optimized_program);

                // print_string_vector(optimized_program);
            } // end is simple loop

            if (is_seek_loop(loop))
            {
                int seek_offset = is_seek_loop(loop);
                // /// cout << seek_offset );
                // // print_string_vector(loop);

                // if (abs(seek_offset) <= 32) // my machine only supports 256bit vectors

                if (seek_flag || optimization_flag)
                    optimized_program = optimize_seek_loop(token, seek_offset, loop, optimized_program);
            } // end is power two

        } // end looping over loop in program list
          // print_string_vector(optimized_program);

        for (int i = 0; i < optimized_program.size(); i++)
        {
            string token = optimized_program[i];
            bf_string_assembler(token);
        }

        // print_string_vector(optimized_program);
    }

    if (!optimization_flag && !seek_flag && !simple_loop_flag)
    {
        // // begin our program compiler loop
        for (int i = 0; i < program_file.size(); i++)
        {
            char ch = program_file[i];
            bf_assembler(ch);
        }
    }

    asm_cleanup();

    // Close the file
    outFile.close();

} // end main()
