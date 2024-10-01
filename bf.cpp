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

bool optimize = false;

/*
jasm, short for josh asm :] outputs to output_file.
*/
void jasm(string text)
{
    *output_file << text << endl;
}

void bf_assembler(char token)
{

    switch (token)
    {
    case '>':
        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // add one to pointer address
        *output_file << "addq    $1, %rax" << endl;
        // Store the adjusted pointer back at -8(%rbp)
        *output_file << "movq    %rax, -8(%rbp)" << endl;
        // tape_position++;
        break;
    case '<':

        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // remove one from pointer address
        *output_file << "subq    $1, %rax" << endl;
        // Store the adjusted pointer back at -8(%rbp)
        *output_file << "movq    %rax, -8(%rbp)" << endl;
        // tape_position--;
        break;
    case '+':

        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // Add 1 to the byte
        *output_file << "addb    $1, %cl" << endl;

        // Store the modified byte back to the address in %rax
        *output_file << "movb    %cl, (%rax)" << endl;

        // tape[tape_position] += 1;
        break;
    case '-':

        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // Decrrement byte in %cl
        *output_file << "subb    $1, %cl" << endl;

        // Store the modified byte back to the address in %rax
        *output_file << "movb    %cl, (%rax)" << endl;

        // tape[tape_position] -= 1;
        break;
    case '.':

        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load the byte from the address into %al (to use with putc)
        *output_file << "movb    (%rax), %al" << endl;

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        *output_file << "movq    stdout(%rip), %rsi" << endl;
        // Move and sign-extend byte in %al to %edi
        *output_file << "movsbl  %al, %edi" << endl;

        // Call putc to print the character
        *output_file << "call    putc@PLT" << endl;

        // cout << tape[tape_position];
        break;
    case ',':

        // Move the file pointer for stdin into the %rdi register
        *output_file << "movq    stdin(%rip), %rdi" << endl;

        // Call the getc function to read a character from stdin (returned in %al)
        *output_file << "call    getc@PLT" << endl;
        // Move the byte from %al into %bl
        *output_file << "movb    %al, %bl" << endl;
        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Store the byte from %bl into the memory pointed to by %rax
        *output_file << "movb    %bl, (%rax)" << endl;

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

        *output_file << start_label << ":" << endl;
        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;
        // jump to matching end label if 0
        *output_file << "cmpb    $0, %cl" << endl;
        *output_file << "je      " << end_label << endl;
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

        *output_file << end_label << ":" << endl;
        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // jump to matching start label if not 0
        *output_file << "cmpb    $0, %cl" << endl;
        *output_file << "jne      " << start_label << endl;
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
    *output_file << ".file	\"bf compiler\"" << endl;

    jasm(".section .data");
    //offset masks= 1,2,4,8,16
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


           
 
    
    
    *output_file << ".text" << endl;
    *output_file << ".section	.text" << endl;
    *output_file << ".globl	main" << endl;
    *output_file << ".type	main, @function" << endl;
    *output_file << endl;

    *output_file << "main:" << endl;

    *output_file << "pushq	%rbp" << endl;
    *output_file << "movq	%rsp, %rbp" << endl;
    // Allocate 16 bytes of stack space for local variables
    *output_file << "subq	$16, %rsp" << endl;
    // Allocate 100,000 bytes with malloc
    *output_file << "movl	$" << to_string(tape_size) << ", %edi" << endl;
    
    *output_file << "call	malloc@PLT" << endl;
    // Store the pointer returned by malloc in the local variable at -8(%rbp)
    *output_file << "movq	%rax, -8(%rbp)" << endl;

    // Calculate the address 50,000 bytes into the allocated memory
    *output_file << "movq    -8(%rbp), %rax" << endl;                            // Load base address into %rax
    *output_file << "addq    $" << to_string(tape_size / 2) << ", %rax" << endl; // Add the offset to %rax

    // Store the adjusted pointer back at -8(%rbp)
    *output_file << "movq    %rax, -8(%rbp)" << endl;
}

void asm_cleanup()
{
    // Set the return value to 0 (successful completion)
    *output_file << "movl    $0, %eax" << endl;
    // Proper stack cleanup
    *output_file << "movq    %rbp, %rsp" << endl;
    // Restore the old base pointer
    *output_file << "popq    %rbp" << endl;
    // Return from the function
    *output_file << "ret" << endl;
    *output_file << endl;

    
        
        
        
        

    
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

    cout << "couldn't find '[' in get_loop_string(), wrong index?? ";
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
    return (n > 0) && ((n & (n - 1)) == 0);
}
/*
Checks if input loop is a seek loop
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
        // cout << expr << endl;
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

void print_padding()
{
    *output_file << endl
                 << endl;
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


void bf_assembler_string(string token)
{
    // print_padding();

    if (token == ">")
    {
        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // add one to pointer address
        *output_file << "addq    $1, %rax" << endl;
        // Store the adjusted pointer back at -8(%rbp)
        *output_file << "movq    %rax, -8(%rbp)" << endl;
        // tape_position++;
    }
    if (token == "<")
    {

        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // remove one from pointer address
        *output_file << "addq    $-1, %rax" << endl;
        // Store the adjusted pointer back at -8(%rbp)
        *output_file << "movq    %rax, -8(%rbp)" << endl;
        // tape_position--;
    }
    if (token == "+")
    {
        // Load base address into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;
        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // Add 1 to the byte
        *output_file << "addb    $1, %cl" << endl;

        // Store the modified byte back to the address in %rax
        *output_file << "movb    %cl, (%rax)" << endl;

        // tape[tape_position] += 1;
    }
    if (token == "-")
    {

        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // Decrrement byte in %cl
        *output_file << "subb    $1, %cl" << endl;

        // Store the modified byte back to the address in %rax
        *output_file << "movb    %cl, (%rax)" << endl;

        // tape[tape_position] -= 1;
    }
    if (token == ".")
    {

        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load the byte from the address into %al (to use with putc)
        *output_file << "movb    (%rax), %al" << endl;

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        *output_file << "movq    stdout(%rip), %rsi" << endl;
        // Move and sign-extend byte in %al to %edi
        *output_file << "movsbl  %al, %edi" << endl;

        // Call putc to print the character
        *output_file << "call    putc@PLT" << endl;

        // cout << tape[tape_position];
    }
    if (token == ",")
    {

        // Move the file pointer for stdin into the %rdi register
        *output_file << "movq    stdin(%rip), %rdi" << endl;

        // Call the getc function to read a character from stdin (returned in %al)
        *output_file << "call    getc@PLT" << endl;
        // Move the byte from %al into %bl
        *output_file << "movb    %al, %bl" << endl;
        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Store the byte from %bl into the memory pointed to by %rax
        *output_file << "movb    %bl, (%rax)" << endl;

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

        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;
        // jump to matching end label if 0
        *output_file << "cmpb    $0, %cl" << endl;
        *output_file << "je      " << end_label << endl;
        *output_file << start_label << ":" << endl;
    }
    if (token == "]")
    {
        int match_loop = myStack.top();
        myStack.pop();
        string start_label = "start_loop_" + to_string(match_loop);
        string end_label = "end_loop_" + to_string(match_loop);
        // Load the pointer from -8(%rbp) into %rax
        *output_file << "movq    -8(%rbp), %rax" << endl;

        // Load byte into %cl (lower 8 bits)
        *output_file << "movb    (%rax), %cl" << endl;

        // jump to matching start label if not 0
        *output_file << "cmpb    $0, %cl" << endl;
        *output_file << "jne      " << start_label << endl;
        *output_file << end_label << ":" << endl;
    }

    if (startsWith(token, "expr_simple:"))
    {

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
        //  address when we begin the loop
        *output_file << "movq    -8(%rbp), %rax" << endl;

        if (sign_of_loop == "-")
            *output_file << "movq    (%rax), %rcx" << endl;

        if (sign_of_loop == "+")
        {
            // input can't be more than 255 so we're good
            *output_file << "movq    $256, %rcx " << endl;
            *output_file << "subq    (%rax), %rcx  " << endl;
        }

        for (const auto &pair : simple_expr)
        {
            if (pair.first == 0)
                continue;

            // pair.first = pointer offset
            // pair.second = cell +- change per loop

            // copy address of the first loop cell
            *output_file << "movq   %rax, %r12" << endl;
            // adjust pointer by our offset
            *output_file << "addq   $" << to_string(pair.first) << ", %r12" << endl;
            // set our constant change of cell
            *output_file << "movq    $" << to_string((pair.second)) << ", %r15" << endl;
            // constant multiplied by times loop happens
            *output_file << "imul   %rcx, %r15" << endl;

            *output_file << "addb    %r15b , (%r12)" << endl;
        }
        // our loop should always end in 0, this assures it, but would break
        // intentional infinite loops ¯\_(ツ)_/¯, saves us like 5 instr per loop
        *output_file << "movb    $0, (%rax)" << endl;
    }

    if (startsWith(token, "expr_seek:"))
    {
        print_padding();
       
        int seek_offset = get_expr_seek_offset(token);

        //cout<< seek_offset<<endl;
   
        seek_loop++;
        string start_label = "start_seek_loop_" + to_string(seek_loop);
        string end_label = "end_seek_loop_" + to_string(seek_loop);

        //jasm("opt:");
        jasm("movq    -8(%rbp), %r8");
        jasm("movb    (%r8), %cl" );
        jasm("cmpb    $0, %cl" );

        jasm( "je      " + end_label );
        //this makes our offset masks easier

        if(seek_offset > 0){
        jasm("addq $1, %r8"); 
        jasm("subq    $32, %r8");
        }
        else{
        jasm("subq $1, %r8"); 
        jasm("addq    $32, %r8");
        }

        // Loop for checking bytes in chunks of 32
        ////////////////////////////////////////////////////////////
        jasm(start_label + ":");

        if(seek_offset > 0)
        jasm("addq    $32, %r8");
        else
        jasm("subq    $32, %r8"); // CHANGE IF neg i think

        
        if(abs(seek_offset) == 4)
        jasm("vmovdqa .four_offset_mask(%rip), %ymm0");
        if(abs(seek_offset) == 1)
        jasm("vmovdqa .one_offset_mask(%rip), %ymm0");
        if(abs(seek_offset) == 2)
        jasm("vmovdqa .two_offset_mask(%rip), %ymm0");
        if(abs(seek_offset) == 8)
        jasm("vmovdqa .eight_offset_mask(%rip), %ymm0");
        if(abs(seek_offset) == 16)
        jasm("vmovdqa .sixteen_offset_mask(%rip), %ymm0");
        if(abs(seek_offset) == 32)
        jasm("vmovdqa .thirty_two_offset_mask(%rip), %ymm0");

        jasm("vpor    (%r8), %ymm0, %ymm0");
        jasm("vpxor   %xmm1, %xmm1, %xmm1");
        jasm("vpcmpeqb        %ymm1, %ymm0, %ymm0");
        jasm("vpmovmskb       %ymm0, %eax");
        jasm("testl   %eax, %eax");
        jasm("je      " + start_label);

        if(seek_offset > 0)
        jasm("rep bsfl    %eax, %eax");  // CHANGE IF neg i think
        else{
            jasm("rep bsrl    %eax, %eax");
            jasm("subq %32, %rax");
        }
        /////////////////////////////////////////////////////////////////
        // save offset
        jasm("addq %rax, %r8");
        jasm("movq    %r8, -8(%rbp)");


        jasm(end_label + ":");
        print_padding();

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

        if (args == "-o")
            optimize = true;
    }

    // Open bf file
    ifstream inputFile(argv[1]);
    if (!inputFile)
    {
        cout << "Couldn't open file: " << argv[1] << endl;
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

    asm_setup();
    print_padding();

    if (optimize)
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

                // print_string_vector(loop);
                optimized_program = optimize_simple_loop(token, loop_increment, loop, optimized_program);

                // print_string_vector(optimized_program);
            } // end is simple loop

            if (is_power_two_seek_loop(loop))
            {
                int seek_offset = is_power_two_seek_loop(loop);
                /// cout << seek_offset << endl;
                // print_string_vector(loop);
                if(abs(seek_offset) <= 32) // my machine only supports 256bit vectors
                optimized_program = optimize_seek_loop(token, seek_offset, loop, optimized_program);
            } // end is power two

        } // end looping over loop in program list
          // print_string_vector(optimized_program);

        for (int i = 0; i < optimized_program.size(); i++)
        {
            string token = optimized_program[i];
            bf_assembler_string(token);
        }

        // print_string_vector(optimized_program);
    }

    if (!optimize)
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
