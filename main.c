/*
 * @Descripttion:  the user interface that an infinite loop that prints the prompt,
 *      gets a line of input, then processes that line of input
 * @version: 1.0
 * @Author: 王远昭 <email: wangyuanzhao35@gmail.com>
 * @Date: 2022-09-12 13:07:55
 * @LastEditors: 王远昭
 * @LastEditTime: 2022-09-23 00:25:00
 */
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<stdint.h> // uint32_t

#define COLUMN_USERNAME_SIZE   32
#define COLUMN_EMAIL_SIZE      255
#define TABLE_MAX_PAGES        100
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)




/****************************************************************************************************************/
//  元命令数据结果，成功或无法识别
typedef enum                    
{ 
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

//  预编译类执行结果，成功、无法识别或语法错误
typedef enum                  
{ 
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT, 
    PREPARE_SYNTAX_ERROR
} PrepareResult; 

//  预编译类类别
typedef enum                
{ 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
} StatementType; 

// SQL语句执行结果
typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;


/**
 * @brief 列数据硬编码(Hard Code)
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
 * @brief 数据库表
 * 
 */
typedef struct 
{
   uint32_t num_rows;
   void* pages[TABLE_MAX_PAGES]; 
} Table;

const uint32_t ID_SIZE          =   size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE    =   size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE       =   size_of_attribute(Row,email);
const uint32_t ID_OFFSET        =   0;
const uint32_t USERNAME_OFFSET  =   ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET     =   USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE         =   ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE        =   4096;
const uint32_t ROWS_PER_PAGE    =   PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS   =   ROWS_PER_PAGE * TABLE_MAX_PAGES;

/**
 * @brief 控制台打印消息
 * 
 */
void print_prompt()
{
    printf("database >");
}

/**
 * @brief 读取用户端输入
 * 
 * @param input_buffer 
 */
void read_input(InputBuffer* input_buffer)
{
    ssize_t bytes_read = 
        getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);
    
    if(bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief 创建输入缓冲流
 * 
 * @return InputBuffer* 
 */
InputBuffer* new_input_buffer()
{
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    if(input_buffer == NULL){printf("Malloc Error!\n");exit(0);}
    
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}


/**
 * @brief 关闭输入缓冲流
 * 
 * @param input_buffer 
 */
void close_input_buffer(InputBuffer* input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}

/**
 * @brief 检查 SQL 命令输入状态
 * 
 * @param input_buffer 
 * @return MetaCommandResult 
 */
MetaCommandResult do_meta_command(InputBuffer* input_buffer)
{
    //由于getline会将字符串的换行也一并包括，这里在exit后也添加上 "\n"
    if(strcmp(input_buffer->buffer,".exit\n") == 0)
    {  
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

/**
 * @brief 检查 SQL 语句可用性
 * 
 * @param input_buffer 
 * @param statement 
 * @return PrepareResult 
 */
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement)
{
    if(strncmp(input_buffer->buffer,"insert\n",6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(
            input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
            statement->row_to_insert.username, statement->row_to_insert.email
        );
        if(args_assigned < 3)
        {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if(strcmp(input_buffer->buffer,"select\n") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * @brief  
 * 
 */
void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num     =   row_num / ROWS_PER_PAGE;
    void*    page         =   table->pages[page_num];
    uint32_t row_offset   =   row_num % ROWS_PER_PAGE;
    uint32_t byte_offset  =   row_offset * ROW_SIZE;
    if(page == NULL)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    return page + byte_offset;
}

void print_row(Row* row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
/**
 * @brief 执行数据库插入命令
 * 
 * @param statement 
 * @param table 
 * @return ExecuteResult 
 */
ExecuteResult execute_insert(Statement *statement, Table* table)
{
    if(table->num_rows >= TABLE_MAX_ROWS)
    {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row
    (
        row_to_insert, 
        row_slot
            (
            table, 
            table->num_rows
            )
    );
    table->num_rows += 1;
    
    return EXECUTE_SUCCESS;
}

/**
 * @brief 执行数据库查看命令
 * 
 * @param Statement 
 * @param table 
 * @return ExecuteResult 
 */
ExecuteResult execute_select(Statement* Statement, Table* table)
{
    Row row;
    for( uint32_t i = 0; i < table->num_rows; i++)
    {
        deserialize_row
        (
            row_slot(table, i),
            &row
        );
        print_row(&row);
    }
    
    return EXECUTE_SUCCESS;
}

/**
 * @brief 执行 SQL 语句
 * 
 * @param statement 
 * @param table 
 * @return ExecuteResult 
 */
ExecuteResult execute_statement(Statement* statement, Table* table)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}

/**
 * @brief 序列化行数据
 * 
 */
void serialize_row(Row* source, void* destination)
{
    memcpy(destination + ID_OFFSET       , &(source->id)        , ID_SIZE       );
    memcpy(destination + USERNAME_OFFSET , &(source->username)  , USERNAME_SIZE );
    memcpy(destination + EMAIL_OFFSET    , &(source->email)     , EMAIL_SIZE    );
}

/**
 * @brief 反序列化行数据
 * 
 */
void deserialize_row(void* source, Row* destination)
{
    memcpy(&(destination->id)        , source + ID_OFFSET       , ID_SIZE       );
    memcpy(&(destination->username)  , source + USERNAME_OFFSET , USERNAME_SIZE );
    memcpy(&(destination->email)     , source + EMAIL_OFFSET    , EMAIL_SIZE    );
}

/**
 * @brief 创建一个空表
 * 
 * @return Table* 
 */
Table* new_table()
{
    Table* table     =   (Table*)malloc(sizeof(Table));
    table->num_rows  =   0;

    for( uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    return table;
}

/**
 * @brief 释放表空间
 * 
 * @param table 
 */
void free_table(Table* table)
{
    for( int i = 0; table->pages[i]; i++)
    {
        free(table->pages[i]);
    }
    free(table);
}

/**
 * @brief 主函数，实现简单的 REPL(Read-Eval-Print-Loop)交互式编程环境 
 * 
 * @param  argc 
 * @param  argv 
 * @return int 
 */
int main(int argc, char* argv[])
{
   InputBuffer* input_buffer  =  new_input_buffer();
   Table* table               =  new_table();
   
   while(true)
   {
        print_prompt();
        read_input(input_buffer);
        if(input_buffer->buffer[0] == '.')
        {
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
        case (PREPARE_SYNTAX_ERROR):
            printf("Syntax Error. Could not parse statement.\n");
            continue;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
            continue;
        }
        
        switch (execute_statement(&statement, table))
        {
            case (EXECUTE_SUCCESS):
                printf("Executed.\n");
                break;
            case (EXECUTE_TABLE_FULL):
                printf("Error:Table full.\n");
                break;
        }
   }
}