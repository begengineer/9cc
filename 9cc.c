#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//　トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM, // 整数トークン
  TK_EOF, // 入力の終わりを表すトークン
} Tokenkind;

typedef struct Token Token;

// トークン型
struct Token{
  Tokenkind kind;
  Token *next;
  int val;
  char *str;
};

// 現在着目しているトークン
Token *token;

// input文字列
char *user_input;

// エラーを報告するための関数
// void error(char *fmt, ...){
//   va_list ap;
//   va_start(ap, fmt);
//   vfprintf(stderr, fmt, ap);
//   fprintf(stderr, "\n");
//   exit(1);
// }
void error_at(char *loc,char *fmt,...){
  va_list ap;
  va_start(ap,fmt);

  int pos = loc - user_input;
  fprintf(stderr,"%s\n",user_input);
  fprintf(stderr,"%*s",pos," ");
  fprintf(stderr, "^ ");
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)return false;
  token = token->next;

  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str,"数ではありません");
  token = token->next;
}

// 次のトークンが整数の場合、トークンを1つ読み進めてその数値を返す。
int expect_number(){
  if(token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(Tokenkind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  // エラー検知用
  char *p = user_input;

  while(*p){
    // 空白文字をスキップ
    if(isspace(*p)){
      p++;
      continue;
    }

    // 記号
    if(*p == '+' || *p == '-'){
      // p++は後置インクリメント
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    // 数字
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p,"トークナイズドできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc,char **argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  token = tokenize();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 式を計算して、その結果をRAXに入れる
  printf("  mov rax, %d\n", expect_number());

  while(!at_eof()){
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  // 終了
  printf("  ret\n");
  return 0;
}