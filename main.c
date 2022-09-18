/*
 * @Descripttion:  the user interface that an infinite loop that prints the prompt,
 *      gets a line of input, then processes that line of input
 * @version: 1.0
 * @Author: 王远昭 <email: wangyuanzhao35@gmail.com>
 * @Date: 2022-09-12 13:07:55
 * @LastEditors: 王远昭
 * @LastEditTime: 2022-09-18 17:52:51
 */
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef unsigned __int32 uint32_t;

/****************************************************************************************************************/
typedef enum{ META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND} MetaCommandResult; //元命令数据结果，成功或无法识别

typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT, PREPARE_SYNTAX_ERROR} PrepareResult; // SQL 执行结果，成功或无法识别

typedef enum{ STATEMENT_INSERT, STATEMENT_SELECT } StatementType; // SQL 语句类别


/**
 * @brief 列数据硬编码(Hart Code)
 * 
 */
typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

/**
 * @brief 输入缓冲流
 * 
 */
typedef struct 
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

/**
 * @brief 预编译 SQL 语句
 * 
 */
typedef struct 
{
    StatementType type;
    Row row_to_insert;      // 只能在 insert statement 中使用
} Statement;

/**
 * @brief 创建输入缓冲流
 * 
 * @return InputBuffer* 
 */
InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    if(input_buffer == NULL){printf("Malloc Error!\n");exit(0);}
    
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

/**
 * @brief 控制台打印消息
 * 
 */
void print_prompt(){
    printf("database >");
}
/**
 * @brief 读取用户端输入
 * 
 * @param input_buffer 
 */
void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = 
        getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);
    
    if(bytes_read <= 0){
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief 关闭输入缓冲流
 * 
 * @param input_buffer 
 */
void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

/**
 * @brief 检查 SQL 命令输入状态
 * 
 * @param input_buffer 
 * @return MetaCommandResult 
 */
MetaCommandResult do_meta_command(InputBuffer* input_buffer){
    //由于getline会将字符串的换行也一并包括，这里在exit后也添加上 "\n"
    if(strcmp(input_buffer->buffer,".exit\n") == 0){  
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
        // printf("Unrecognized command :%s",input_buffer->buffer);
    }
}

/**
 * @brief 检查 SQL 语句可用性
 * 
 * @param input_buffer 
 * @param statement 
 * @return PrepareResult 
 */
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement){
    if(strncmp(input_buffer->buffer,"insert\n",6) == 0){
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(
            input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
            statement->row_to_insert.username, statement->row_to_insert.email
        );
        if(args_assigned < 3){
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if(strcmp(input_buffer->buffer,"select\n") == 0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * @brief 执行 SQL 语句
 * 
 * @param statement 
 */
void execute_statement(Statement* statement){
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        printf("This is where we would an insert.\n");
        break;
    case (STATEMENT_SELECT):
        printf("This is where we would do a select.\n");
    default:
        break;
    }
}

/**
 * @brief 主函数，实现简单的 REPL(Read-Eval-Print-Loop)交互式编程环境 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char* argv[]){
   InputBuffer* input_buffer = new_input_buffer();
   while(true){
        print_prompt();
        read_input(input_buffer);
        if(input_buffer->buffer[0] == '.'){
            switch (do_meta_command(input_buffer))
            {
            case (META_COMMAND_SUCCESS):  // 这里暂时不会出现这种情况
                break;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Unrecognized command %s",input_buffer->buffer);
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer,&statement))
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
            continue;
        }

        execute_statement(&statement);
        printf("Executed.\n");
   }
}