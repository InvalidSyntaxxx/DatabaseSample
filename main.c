/*
 * @Descripttion:  the user interface that an infinite loop that prints the prompt,
 *      gets a line of input, then processes that line of input
 * @version: 1.0
 * @Author: 王远昭 <email: wangyuanzhao35@gmail.com>
 * @Date: 2022-09-12 13:07:55
 * @LastEditors: 王远昭
 * @LastEditTime: 2022-09-12 13:53:33
 */
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

typedef struct 
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

 InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    if(input_buffer == NULL){printf("Malloc Error!\n");exit(0);}
    
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
 }

 void print_prompt(){
    printf("database >");
 }

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = 
        getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);
    
    if(bytes_read <= 0){
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
}
void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(int argc, char* argv[]){
   InputBuffer* input_buffer = new_input_buffer();
   printf("%d",strcmp("exit","exit"));
   while(true){
        print_prompt();
        read_input(input_buffer);
        if(strcmp(input_buffer->buffer,".exit\n") == 0){  //由于getline会将字符串的换行也一并包括，这里在exit后也添加上 "\n"
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }else{
            printf("Unrecognized command :%s",input_buffer->buffer);
        }
   }
}