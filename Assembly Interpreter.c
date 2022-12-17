#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOV 0
#define INC 1
#define DEC 2
#define ADD 3
#define SUB 4
#define MUL 5
#define DIV 6
#define LBL 7
#define JMP 8
#define CMP 9
#define JNE 10
#define JE  11
#define JGE 12
#define JG  13
#define JLE 14
#define JL  15
#define CLL 16
#define RET 17
#define MSG 18
#define END 19

#define MAX_LBL 20
#define MAX_MSG 50
#define NUM_OPS 20
#define MAX_CODE_LEN 200

#define INVALID_CHAR (*program == ' ' || *program == ';' || *program == '\n' || *program == '\t')
#define INVALID_CHAR_DP (**program == ' ' || **program == ';' || **program == '\n' || **program == '\t')

struct instruction {
    short opcode;
    char opcodeString[MAX_LBL];
  
    int line;
    int value;
  
    char toRegister;
    char fromRegister;
    
    char lbl[MAX_LBL]; 
    char message[MAX_MSG];
    char code[MAX_CODE_LEN];
  
    int compareX;
    int compareY;
    
};
typedef struct instruction Instruction;

struct function {
    char lbl[MAX_LBL];
    int numRoutines;
    Instruction subroutine[10];
};
typedef struct function Function;

int registers[26] = {0};
short cmpX;
short cmpY;
short comparator;
short validEnd = 1;
char msg[MAX_MSG] = {0};
char* formattedMsg;
const char* operations[] = {"mov", "inc", "dec", "add", "sub", "mul", "div", "label:", "jmp", "cmp", "jne", "je", "jge", "jg", "jle", "jl", "call", "ret", "msg", "end"};
Function functions[10];
int numFunctions = 0;

Instruction* createInstr() {
    Instruction* instr = calloc(1, sizeof(Instruction));
    instr->opcode = -1;
    return instr;
}

//function prototypes
void printRegisters(void);
void removeComment(const char** program);
void trimLine(const char** program);
void printTokens(Instruction* token, int numTokens);
void printFunctions(Function* function);
void parseOpcode(char* string, Instruction* instr);
void parseMov(const char** program, Instruction* instr);
void parseMath(const char** program, Instruction* instr);
void parseJmp(const char** program, Instruction* instr);
void parseMsg(const char** program, Instruction* instr);
void parseEnd(void);
void parseLbl(const char** program, char* string, Function* functions);
void parseCall(const char** program, Instruction* instr);
void parseCmp(const char** program, Instruction* instr);
void parseRet(Instruction* instr);
void lexer(const char* program, Instruction* tokenizedProgram, int* numTokens, Function* functions);
void executeMathOp(Instruction* instr);
void executeCall(Instruction* instr);
void executeCmp(Instruction* instr);
void executeJmp(Instruction** instr, int* pgmCounter);
void executor(Instruction* tokenizedProgram, int* numTokens);

//main program driver.
char* assembler_interpreter (const char* program) {

    numFunctions = 0;
    validEnd = 1;
    memset(functions, 0, sizeof(Function) * 10);
    memset(msg, 0, sizeof(char) * MAX_MSG);
    memset(registers, 0, sizeof(int) * 26);
       
    Instruction* tokenizedProgram = calloc(50, sizeof(Instruction));
    int numTokens = 0;
    formattedMsg = calloc(MAX_MSG, sizeof(char));

    lexer(program, tokenizedProgram, &numTokens, functions);
      
    executor(tokenizedProgram, &numTokens);
    free(tokenizedProgram);
  
    if (validEnd != -1) {
        return formattedMsg;
    }
  
    return (char*) -1;
}


void printRegisters(void) {
    for (int i = 0; i < 26; i++) {
        printf("%d, ", registers[i]);
    }
}

//admin functions to clean newlines, blank spaces, unused characters.
void removeComment(const char** program) {
    while (**program != '\0' && **program != '\n') (*program)++;
}

void trimLine(const char** program) {
    while (INVALID_CHAR_DP) {
        if (**program == ';') {
            removeComment(program);
            return;
        }
        (*program)++;
    }    
}

//print the tokens produced by lexer to verify correct lexical analysis.
void printTokens(Instruction* token, int numTokens) {
    for (int i = 0; i < numTokens; i++, token++) {
        printf("Opcode: %d\n"
               "Opcode: %s\n"
               "Value: %d\n"
               "ToRegister: %c\n"
               "FromRegister: %c\n"
               "Label: %s\n"
               "Code: %s\n"
               "Msg: %s\n"
               "//////////////////\n",
               token->opcode,
               token->opcodeString,
               token->value,
               token->toRegister,
               token->fromRegister,
               token->lbl,
               token->code,
               msg);
    }
}

//print functions created by lexer to verify correct lexical analysis.
void printFunctions(Function* func) {
  
    for (int i = 0; i < numFunctions; func++, i++) {
        printf("\nFunction Label: %s\n"
               "Number of tokens: %d\n"
               "Printing tokens:\n\n",
               func->lbl, func->numRoutines);
        printTokens(func->subroutine, func->numRoutines);
    }
}

void printToken(Instruction* token) {
    printf("Opcode: %d\n"
               "Opcode: %s\n"
               "Value: %d\n"
               "ToRegister: %c\n"
               "FromRegister: %c\n"
               "Label: %s\n"
               "Code: %s\n"
               "Msg: %s\n"
               "//////////////////\n",
               token->opcode,
               token->opcodeString,
               token->value,
               token->toRegister,
               token->fromRegister,
               token->lbl,
               token->code,
               msg);
}

//parses the opcode of the instruction and fills in the instruction data structure.
void parseOpcode(char* string, Instruction* instr) {
    for (int i = 0; i < NUM_OPS; i++) {
        if (strcmp(string, operations[i]) == 0) {
          instr->opcode = i;
          strncpy(instr->opcodeString, operations[i], (size_t)MAX_LBL);
          return;
        }
    }
}

//next parsing functions are called by the lexer once it verifies the opcode based on the rules.
void parseMov(const char** program, Instruction* instr) {
    char temp[MAX_LBL];
    int currentValue = 0;
    int negative = 1;
    
    instr->toRegister = **program;
    (*program)++;
    while (INVALID_CHAR_DP || **program == ',') (*program)++;
  
    if (**program == '-') {
      negative = -1;
      (*program)++;
    }
  
    for (int i = 0; i < MAX_LBL - 1 && !INVALID_CHAR_DP; i++, (*program)++) {
        temp[i] = **program;
        temp[i + 1] = '\0';
    }
  
    if (temp[0] >= '0' && temp[0] <= '9') {
        for (int i = 0; temp[i] != '\0'; i++) {
            currentValue *= 10;
            currentValue += temp[i] - '0';
        }
        instr->value = currentValue * negative;
    } else {
        instr->fromRegister = temp[0];
    }
    
}

void parseMath(const char** program, Instruction* instr) {
    int currentValue = 0;
    int negative = 1;
    if (**program == '-') {
      negative = -1;
      (*program)++;
    }
    
    if (**program >= 'a' && **program <= 'z') instr->toRegister = **program;
    (*program)++;
    if (instr->opcode == INC || instr->opcode == DEC) {
        (*program)++;
        return;
    }
    while (INVALID_CHAR_DP || **program == ',') (*program)++;
  
    if (**program == '-') {
      negative = -1;
      (*program)++;
    }
  
    if (**program >= 'a' && **program <= 'z') {
        instr->fromRegister = **program;
        (*program)++;
    } else {
        while (**program >= '0' && **program <= '9') {
            currentValue *= 10;
            currentValue += **program - '0';
            (*program)++;
        }
        instr->value = currentValue * negative;
    }  
}

void parseJmp(const char** program, Instruction* instr) {
    char label[MAX_LBL] = {0};

    for (int i = 0; !INVALID_CHAR_DP && i < MAX_LBL; i++, (*program)++) {
        label[i] = **program;
    }
    (*program)++;
    strncpy(instr->opcodeString, operations[instr->opcode], 4 * sizeof(char));
    strncpy(instr->lbl, label, (size_t)MAX_LBL);
}

void parseMsg(const char** program, Instruction* instr) {
    char tempMessage[MAX_MSG] = {0};
    
    for (int i = 0; i < MAX_MSG && **program != '\n' && **program != '\0'; i++, (*program)++) {
        if (**program == ';') {
            removeComment(program);
            tempMessage[i] = '\n';
            tempMessage[i + 1] = '\0';
            break;
        }
        tempMessage[i] = **program;
        tempMessage[i + 1] = '\0';
    }
    strncpy(instr->message, tempMessage, (size_t)MAX_MSG);
}

void parseLbl(const char** program, char* string, Function* functions) {    
    char instructions[MAX_CODE_LEN] = {0};
    string[strlen(string) - 1] = '\0';
    
    int i = 0;
    
    Function* func = calloc(1, sizeof(Function));
    
    for (; i < MAX_CODE_LEN;) {
      
        if (**program == '\n' || **program == '\0') break;
        
        while(**program != '\n' && **program != '\0') {
          
            if (**program == ';') {
                removeComment(program);
                break;
            }
            instructions[i++] = **program;
            (*program)++;
        }
        instructions[i++] = **program;
        if (**program == '\0') break;
      
        if ( (*(*program + 1) != ' ' && *(*program + 2) != ' ' && *(*program + 3) != ' ' && *(*program + 4) != ' ') && *(*program + 1) != '\t') break;
        (*program)++;
    }
    instructions[i] = '\0';
    
    strncpy(func->lbl, string, (size_t)MAX_LBL);
    if (strlen(instructions) > 1) {
      lexer(instructions, func->subroutine, &func->numRoutines, functions);  
      memcpy(&functions[numFunctions], func, sizeof(Function));
      numFunctions++;
    }
    
}

void parseCall(const char** program, Instruction* instr) {
    char label[MAX_LBL] = {0};
    
    for (int i = 0; i < MAX_LBL && !INVALID_CHAR_DP; i++, (*program)++) {
        label[i] = **program;
        label[i + 1] = '\0';
    }
   
    strncpy(instr->lbl, label, (size_t)MAX_LBL);
}

void parseCmp(const char** program, Instruction* instr) {
    int negative = 1;
    int tempX = 0;
    int tempY = 0;
  
    if (**program >= 'a' && **program <= 'z') {
        tempX = registers[**program - 'a'];
        instr->toRegister = **program;
        (*program)++;
    } else {
        if (**program == '-') {
            negative = -1;
            (*program)++;
        }
        while (**program != ',') {
            tempX *= 10;
            tempX += **program - '0';
            (*program)++;
        }
        instr->compareX = tempX * negative;
    }
  
    while (INVALID_CHAR_DP || **program == ',') (*program)++;
    negative = 1;
    if (**program >= 'a' && **program <= 'z') {
        tempY = registers[**program - 'a'];
        instr->fromRegister = **program;
        (*program)++;
      
    } else {
      
        if (**program == '-') {
            negative = -1;
            (*program)++;
        }
      
        while (!INVALID_CHAR_DP) {
            tempY *= 10;
            tempY += **program - '0';
            (*program)++;
        }
        instr->compareY = tempY * negative;
    }    
    
    strcpy(instr->opcodeString, "cmp");
}

void parseRet(Instruction* instr) {
    instr->opcode = RET;
    strcpy(instr->opcodeString, operations[RET]);
}

//the actual lexical analyzer that creates and fills in tokens
void lexer(const char* program, Instruction* tokenizedProgram, int* nTokens, Function* functions) {
    int MAXLEN = MAX_LBL;
    Instruction* instr;
    Instruction* tokenPtr = tokenizedProgram;
    char string[MAXLEN];
    
 
    while (*program != '\0') {
        memset(string, 0, sizeof(char) * MAXLEN);

        trimLine(&program);   
        if (*program == '\0') return;
      
        instr = createInstr();
        for (int i = 0; i < MAXLEN - 1 && !INVALID_CHAR; i++) {
            string[i] = *program;
            program++;
        }

        if (strcmp(string, "\0") == 0) continue;
        parseOpcode(string, instr);
        if (instr->opcode != -1) {
            if (*program == '\0') return;
            trimLine(&program);
        }
       
        switch(instr->opcode) {
            
            case MOV:
              parseMov(&program, instr);
              break;
            
            case INC:
              parseMath(&program, instr);
              break;
            
            case DEC:
              parseMath(&program, instr);
              break;
            
            case ADD:
              parseMath(&program, instr);
              break;
            
            case SUB:
              parseMath(&program, instr);
              break;
              
            case MUL:
              parseMath(&program, instr);
              break;
            
            case DIV:
              parseMath(&program, instr);
              break;
            
            case MSG:
              parseMsg(&program, instr);
              break;
            
            case END:
              break;
            
            case CMP:
              parseCmp(&program, instr);
              break;
            
            case CLL:
              parseCall(&program, instr);
              break;
            
            case RET:
              parseRet(instr);  
              program++;
              break;
            
            case JMP:
              parseJmp(&program, instr);
              break;
            
            case JNE:
              parseJmp(&program, instr);
              break;
            
            case JE:
              parseJmp(&program, instr);
              break;
            
            case JGE:
              parseJmp(&program, instr);
              break;
            
            case JG:
              parseJmp(&program, instr);
              break;
            
            case JLE:
              parseJmp(&program, instr);
              break;
            
            case JL:
              parseJmp(&program, instr);
              break;
            
            default:
              instr->opcode = LBL;
              if ( (*(program + 1) == ' ' && *(program + 2) == ' ' && *(program + 3) == ' ' && *(program + 4) == ' ') || *(program + 1) == '\t'){
                trimLine(&program);
                parseLbl(&program, string, functions);
              }            
        }
      
        
        if (instr->opcode != LBL && instr->opcode != -1) {
            (*nTokens)++; 
            memcpy(tokenPtr, instr, sizeof(Instruction));
            tokenPtr++;          
        }
   }    

}

//execute operations are called based on the type of opcode found in each instruction data structure. 
//these functions will execute the actual instructions based on the passed instruction data structure. 
void executeMov(Instruction* instr) {
    if (instr->fromRegister != '\0') {
        registers[instr->toRegister - 'a'] = registers[instr->fromRegister - 'a'];
    } else {
        registers[instr->toRegister - 'a'] = instr->value;
    }
}

void executeMathOp(Instruction* instr) {
  
    switch(instr->opcode) {
        
        case INC:
          registers[instr->toRegister - 'a'] += 1;
          break;
        
        case DEC:
          registers[instr->toRegister - 'a'] -= 1;
          break;
        
        case DIV:
          if (instr->fromRegister != '\0') {
              registers[instr->toRegister - 'a'] /= registers[instr->fromRegister  - 'a'];
          } else {
              registers[instr->toRegister - 'a'] /= instr->value;
          }
          break;
        
        case MUL:
          if (instr->fromRegister != '\0') {
              registers[instr->toRegister - 'a'] *= registers[instr->fromRegister  - 'a'];
          } else {
              registers[instr->toRegister - 'a'] *= instr->value;
          }
          break;
        
          case ADD:
          if (instr->fromRegister != '\0') {
              registers[instr->toRegister - 'a'] += registers[instr->fromRegister  - 'a'];
          } else {
              registers[instr->toRegister - 'a'] += instr->value;
          }
          break;        
        
        case SUB:
          if (instr->fromRegister != '\0') {
              registers[instr->toRegister - 'a'] -= registers[instr->fromRegister  - 'a'];
          } else {
              registers[instr->toRegister - 'a'] -= instr->value;
          }
          break;  
    }
}

void executeCall(Instruction* instr) {
    
    Instruction* calledPtr;
    for (int i = 0; i < numFunctions; i++) {
        if (strcmp(instr->lbl, functions[i].lbl) == 0) {
            calledPtr = functions[i].subroutine;
            executor(calledPtr, &functions[i].numRoutines);
            return;
        }
    }
}

void executeCmp(Instruction* instr) {
    if (instr->toRegister != '\0') {
        cmpX = registers[instr->toRegister - 'a'];
    } else {
        cmpX = instr->compareX;
    }
  
    if (instr->fromRegister != '\0') {
        cmpY = registers[instr->fromRegister - 'a'];
    } else {
        cmpY = instr->compareY;
    }
}

void executeJmp(Instruction** instr, int* pgmCounter) {
      
    if ((*instr)->opcode == JNE && cmpX != cmpY)       comparator = 1;
    else if ((*instr)->opcode == JE && cmpX == cmpY)   comparator = 1;
    else if ((*instr)->opcode == JGE && cmpX >= cmpY)  comparator = 1;
    else if ((*instr)->opcode == JG && cmpX > cmpY)    comparator = 1;
    else if ((*instr)->opcode == JLE && cmpX <= cmpY)  comparator = 1;
    else if ((*instr)->opcode == JL && cmpX < cmpY)    comparator = 1;
    else if ((*instr)->opcode == JMP)                  comparator = 1; 
    else                                               comparator = 0;
    
    if (comparator == 1) {
        for (int i = 0; i < numFunctions; i++) {
            if (strcmp((*instr)->lbl, functions[i].lbl) == 0) {
                *instr = functions[i].subroutine;
                *pgmCounter = 0;
                return;
            }
        }
    }
    (*instr)++;   
}

void executeRet(int* returnFlag) {
    *returnFlag = 1;
}

void executeMsg(Instruction* instr) {
    memset(formattedMsg, 0, sizeof(char) * MAX_MSG);
    char* msgPtr = instr->message;
    char* formMsgPtr = formattedMsg; 
    
    for (int i = 0; i < strlen(instr->message); i++) {
        while (*msgPtr == ' ' || *msgPtr == ',') {
            msgPtr++;
            i++;
        }
        if (*msgPtr == '\0' || *msgPtr == '\n') return;
      
        if (*msgPtr == '\'') {
            msgPtr++;
            i++;
            while (*msgPtr != '\'' && *msgPtr != '\0') {
                *formMsgPtr = *msgPtr; 
                formMsgPtr++;
                msgPtr++;
                i++;
            }
            msgPtr++;
        } else {
            sprintf(formMsgPtr, "%d", registers[*msgPtr - 'a']);
            while(*formMsgPtr != '\0') formMsgPtr++;
            msgPtr++;
        }      
    }
}

//the actual driver that moves through the list of tokens and calls the respective execute functions based on the instruction data structure opcode.
void executor(Instruction* tokenizedProgram, int* numTokens) {
    Instruction* instrPtr = tokenizedProgram;
    int pgmCounter = 0;
  
    while (pgmCounter++ <= *numTokens) {

        if (pgmCounter == *numTokens && (instrPtr->opcode != END && instrPtr->opcode != RET &&
                                         instrPtr->opcode != JMP && instrPtr->opcode != CLL)) {
            validEnd = -1;
            return;     
        }     
      
        switch(instrPtr->opcode) {
            
             case MOV:
                executeMov(instrPtr);
                instrPtr++;
                break;
             case INC:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case DEC:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case ADD:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case SUB:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case MUL:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case DIV:
                executeMathOp(instrPtr);
                instrPtr++;
                break;
             case JMP:
                executeJmp(&instrPtr, &pgmCounter);
                break;
             case JNE:
                executeJmp(&instrPtr, &pgmCounter);
                break;            
             case JE:
                executeJmp(&instrPtr, &pgmCounter);
                break;              
             case JGE:
                executeJmp(&instrPtr, &pgmCounter);
                break;            
             case JG:
                executeJmp(&instrPtr, &pgmCounter);
                break;            
             case JLE:
                executeJmp(&instrPtr, &pgmCounter);
                break;            
             case JL:
                executeJmp(&instrPtr, &pgmCounter);
                break;
             case CLL:
                executeCall(instrPtr);
                instrPtr++;
                break;            
             case MSG:
                executeMsg(instrPtr);
                instrPtr++;
                break;            
             case RET:
                return;           
             case CMP:
                executeCmp(instrPtr);
                instrPtr++;
                break;            
             case END:
                validEnd *= 1;
                return;
        }                                  
    }
} 