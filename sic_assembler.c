#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int addr;
    char *label;
    char *opcode;
    char *operand;    
}Instruction;

typedef struct {
    char *symbol;
    int locctr;
}Symtable;

typedef struct {
    char name[10];
    char code[3];
}OPtable;

Symtable symtable[50];
OPtable optable[60];
int program_length;
int end_addr;

int initOPtable(){
    char readLine[1000] = {0};
    int line = 0;

    FILE *op = fopen("opcode.txt", "r");
	if(NULL == op)
	{
		printf("failed to open dos.txt\n");
		return 1;
	}

    while(!feof(op))
	{
        char opName[10];
        char code[3];
        
		memset(readLine, 0, sizeof(readLine));
		fgets(readLine, sizeof(readLine) - 1, op); 
        
        sscanf(readLine, "%s %s", opName, code);

        strcpy(optable[line].name, opName);
        strcpy(optable[line].code, code);
        // printf("opName %s, code %s \n", opName, code);
        line++;
        
	}
	fclose(op);
}

int if_symbol_exist(char *symbol){
    int i;
    for(i = 0; i < 50; i++){
        
        if(symtable[i].symbol == NULL)
            break;
        else{
            // printf("symtable %d %s \n", i, symtable[i].symbol);
            if(0 == strcmp(symtable[i].symbol, symbol))
                return 0;
        }
    }
    return -1;
}

int search_symbol_addr(char *symbol){
    int i;
    char *temp = malloc(strlen(symbol) + 1);
    strcpy(temp, symbol);
    temp = strtok(temp, ",");

    for(i = 0; i < 50; i++){
        // printf("symtable %d symbol %s\n", i, symtable[i].symbol);
        if(symtable[i].symbol == NULL)
            break;
        if(0 == strcmp(symtable[i].symbol, temp))
            return symtable[i].locctr;
    }
    return -1;
}

char* search_opcode(char *opcode){
    int i;
    for(i = 0; i < 60; i++){
        // printf("optable %s code %s\n", optable[i].name, optable[i].code);
        if(0 == strcmp(optable[i].name, opcode))
            return optable[i].code;
    }
    return NULL;
}

void output_symtable(){
    FILE *output = fopen("symbol_table.txt","w");
    int i = 0;
    for(i = 0; i < 50; i++){
        
        if(symtable[i].symbol == NULL)
            break;
        else{
            // printf("symtable %d %s \n", i, symtable[i].symbol);
            fprintf(output, "%s\t%X\n", symtable[i].symbol, symtable[i].locctr);
        }
    }
    fclose(output);
}

void pass_one(){

    FILE *source = fopen("source.txt", "r");
    FILE *output = fopen("p1_intermediate.txt","w");
    char readLine[1000] = {0};
    int line = 0;
    int start_addr = 0;
    int pc = 0;
    int locctr = 0;
    int sym_counter = 0; //symbol table counter
    Instruction instruction;

	if(NULL == source)
	{
		printf("failed to open source.txt\n");
		exit(1);
	}

    while(!feof(source))
	{
        // int i = 0;
        char *command[3];
        
		memset(readLine, 0, sizeof(readLine));
		fgets(readLine, sizeof(readLine) - 1, source); // read line from source file
        
        // parse the data from source file
        char *p = strtok(readLine, "\r\n");  // remove \r\n
        p = strtok(p, "\t"); 
        int tokenN = 0;
        while (p != NULL) {
            command[tokenN] = p;
            // printf("%s \n", command[tokenN]);
            p = strtok(NULL, "\t");
            tokenN++;
        }
        // printf("tokenN %d\n", tokenN);

        switch(tokenN){
            case 1:
                instruction.label = "";
                instruction.opcode = command[0];
                instruction.operand = "";
                break;
            case 2:
                instruction.label = "";
                instruction.opcode = command[0];
                instruction.operand = command[1];
                break;
            case 3:
                instruction.label = command[0];
                instruction.opcode = command[1];
                instruction.operand = command[2];
                break;
            case 0:
                fclose(source);
                fclose(output);
                return;
        }

        // printf("label %s, opcode %s, operand %s\n", instruction.label, instruction.opcode, instruction.operand);

        if(line == 0){
            if(0 == strcmp("START", instruction.opcode)){  // if OPCODE = START
                start_addr = strtol(instruction.operand, NULL, 16); //save #[OPERAND] as starting address
                pc = start_addr;  //initialize LOCCTR to starting address       
            }
            else
                pc = 0; //initialize LOCCTR to 0
        
            fprintf(output, "%X\t%s\t%s\t%s\r\n", pc, instruction.label, instruction.opcode, instruction.operand); //write line to intermediate file
        }
        else{
            // printf("label %s, opcode %s, operand %s\n", instruction.label, instruction.opcode, instruction.operand);
            if(0 != strcmp("END", instruction.opcode)){
                if(readLine[0] != '.'){
                    if(0 != strcmp("", instruction.label)){
                        // printf("label %s \n",instruction.label);
                        //search SYMTAB for LABEL
                        if(if_symbol_exist(instruction.label) != 0){ //if found
                            // insert (LABEL, LOCCTR) into SYMTAB
                            symtable[sym_counter].symbol = malloc(strlen(instruction.label) + 1);
                            strcpy(symtable[sym_counter].symbol, instruction.label);
                            symtable[sym_counter].locctr = pc;
                            sym_counter++;
                        }
                    }
                    // search OPTAB for OPCODE
                    if(NULL != search_opcode(instruction.opcode)){ //if found in OPTAB
                        locctr = pc;
                        pc += 3;
                    }
                    else if(0 == strcmp("WORD", instruction.opcode)){ // if opcode is WORD
                        locctr = pc;
                        pc += 3;
                    }
                    else if(0 == strcmp("RESW", instruction.opcode)){ // if opcode is RESW
                        locctr = pc;
                        pc += 3 * atoi(instruction.operand);
                    }
                    else if(0 == strcmp("RESB", instruction.opcode)){ // if opcode is RESB
                        locctr = pc;
                        pc += atoi(instruction.operand);
                    }
                    else if(0 == strcmp("BYTE", instruction.opcode)){ // if opcode is BYTE
                        char *temp = malloc(strlen(instruction.operand) + 1);
                        strcpy(temp, instruction.operand);
                        strtok(temp, "\'");
                        locctr = pc;
                        switch(instruction.operand[0]){
                            case 'C':
                            case 'c':
                                pc += strlen(strtok(NULL, "\'"));
                                break;
                            case 'X':
                            case 'x':
                                pc++;
                                break;
                        }
                    }
                }
                fprintf(output, "%X\t%s\t%s\t%s\r\n", locctr, instruction.label, instruction.opcode, instruction.operand); //write line to intermediate file
                // printf("write locctr %x for %s\n", locctr, instruction.opcode);
            }
            else{
                fprintf(output, "\t%s\t%s\t%s\r\n", instruction.label, instruction.opcode, instruction.operand); //write last line to intermediate file
                program_length = pc - start_addr; // save (LOCCTR - starting address) as program length
                end_addr = pc;
            }
            
        }
        line++;
	}
	fclose(source);
    fclose(output);
}

void pass_two(){
    FILE *source = fopen("p1_intermediate.txt", "r");
    FILE *output = fopen("p2_instruction_list.txt", "w");
    FILE *final = fopen("object_program.txt", "w");

    char readLine[1000] = {0};
    int line = 0;
    int opcode;
    int aym_addr;
    int operand_addr;
    int object_code;
    char object_code_str[10];
    int text_record_count = 0;
    char text_record[128];
    int first_addr = 0;
    int start_addr = 0;
    int next_flag = 0;
    Instruction instruction;

    if(NULL == source)
	{
		printf("failed to open intermediate.txt\n");
		exit(1);
	}

    while(!feof(source))
	{
        char *command[4];
        
		memset(readLine, 0, sizeof(readLine));
		fgets(readLine, sizeof(readLine) - 1, source); // read line from intermediate file
        
        char *p = strtok(readLine, "\r\n"); // remove \r\n
        p = strtok(p, "\t");
        int tokenN = 0;
        while (p != NULL) {
            command[tokenN] = p;
            p = strtok(NULL, "\t");
            tokenN++;
        }

        switch(tokenN){
            case 2:
                if(0 == strcmp("END", command[0])){
                    // instruction.addr = "";
                    instruction.label = "";
                    instruction.opcode = command[0];
                    instruction.operand = command[1];
                }
                else{
                    instruction.addr = strtol(command[0], NULL, 16);
                    instruction.label = "";
                    instruction.opcode = command[1];
                    instruction.operand = "";
                }
                break;
            case 3:
                instruction.addr = strtol(command[0], NULL, 16);
                instruction.label = "";
                instruction.opcode = command[1];
                instruction.operand = command[2];
                break;
            case 4:
                instruction.addr = strtol(command[0], NULL, 16);
                instruction.label = command[1];
                instruction.opcode = command[2];
                instruction.operand = command[3];
                break;
            case 0:
                fclose(source);
                fclose(output);
                fclose(final);
                return;
        }

        if(line == 0){
            if(0 == strcmp("START", instruction.opcode))
                fprintf(output, "%X\t%s\t%s\t%s\r\n", instruction.addr, instruction.label, instruction.opcode, instruction.operand); //Write first to instruction list file
            fprintf(final, "H^%s\t^%06X^%06X\r\n",instruction.label, instruction.addr, program_length); //Write header record to object program
            // fprintf(final, "T^%06X^", instruction.addr);
            start_addr = instruction.addr;
            first_addr = instruction.addr;
        }
        else{
            
            if(0 != strcmp("END", instruction.opcode)){ //if opcode not END
                if(readLine[0] != '.'){ // if not a comment
                    char *temp = search_opcode(instruction.opcode);
                    if(temp != NULL){
                        opcode = strtol(temp, NULL, 16) << 16;
                        if(0 != strcmp("", instruction.operand)){
                            int symbol_addr = search_symbol_addr(instruction.operand); //search the address by operand

                            if(symbol_addr != -1){ // compute the address
                                if(strstr(instruction.operand, ",X") != NULL)
                                    operand_addr = symbol_addr + 32768;
                                else
                                    operand_addr = symbol_addr;
                            }
                            else
                                operand_addr = 0;
                        }
                        else
                            operand_addr = 0;
                        object_code = opcode + operand_addr; // compute object code
                        sprintf(object_code_str, "%06X", object_code);
                        
                    }
                    else if(0 == strcmp("WORD", instruction.opcode)){ // if opcode is WORD
                        object_code = atoi(instruction.operand);
                        sprintf(object_code_str, "%06X", object_code);

                    }
                    else if(0 == strcmp("BYTE", instruction.opcode)){ // if opcode is BYTE
                        char *data;
                        int i;
                        char *temp = malloc(strlen(instruction.operand) + 1);
                        strcpy(temp, instruction.operand);
                        strtok(temp, "\'");
                        switch(instruction.operand[0]){
                            case 'C':
                            case 'c':
                                data = strtok(NULL, "\'");
                                for(i = 0; i < strlen(data); i++){
                                    object_code = object_code << 8;
                                    object_code += data[i];
                                }
                                sprintf(object_code_str, "%X", object_code);
                                break;
                            case 'X':
                            case 'x':
                                strcpy(object_code_str, strtok(NULL, "\'"));
                                break;
                        }
                    }
                    
                    if(0 == strcmp("RESW", instruction.opcode) || 0 == strcmp("RESB", instruction.opcode)){ //if opcode is RESW or RESB
                        strcpy(object_code_str,"");
                        next_flag++;
                    }
                    else
                        next_flag = 0;

                    if(text_record_count == 10 || next_flag == 1){ //if object code count is 10 or opcode is RESW or opcode is RESB
                        fprintf(final, "%02X^%s\n", (instruction.addr - start_addr), text_record); //write the text record to object program
                        text_record_count = 0;
                    }

                    if(0 != strcmp("", object_code_str)){
                        if(text_record_count == 0 && next_flag == 0){ 
                            fprintf(final, "T^%06X^", instruction.addr); // init the new text record
                            start_addr = instruction.addr;
                            strcpy(text_record, object_code_str);
                        }else{
                            strcat(text_record, "^");  
                            strcat(text_record, object_code_str); // add the object code to text record
                        }
                        text_record_count++;
                    }
                }
                fprintf(output, "%X\t%s\t%s\t%-8s\t%s\r\n", instruction.addr, instruction.label, instruction.opcode, instruction.operand, object_code_str); //write line to list file
                // printf("object code %s\n",object_code_str);
            }
            else{
                fprintf(final, "%02X^%s\n", (end_addr - start_addr), text_record); //write the last text record object program
                fprintf(final, "E^%06X", first_addr);  //write the last line to object program
                fprintf(output, "\t%s\t%s\t%s\r\n", instruction.label, instruction.opcode, instruction.operand); //write last line to list file

            }
        }
        line++;
        
	}
	fclose(source);
    fclose(output);
    fclose(final);
}

int main(void){

    initOPtable(); // read the OPTAB from the opcode.txt
    pass_one(); 
    output_symtable();
    pass_two();

	return 0;
}