#include "pool.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct pool{
  char *data;
  int *front;
  int *back;
  int size;
  int isize;
};

struct pool *pool_create(int size){
  struct pool *p = malloc(sizeof(struct pool));
  p->data = malloc(sizeof(char) * size);
  p->front = malloc(sizeof(int) * 100000);
  p->back = malloc(sizeof(int) * 100000);
  p->size = size;
  p->isize = 0;
  return p;
}

bool pool_destroy(struct pool *p){
  if(p->isize) return false;
  free(p->data);
  free(p->front);
  free(p->back);
  free(p);
  return true;
}

char *pool_alloc(struct pool *p, int size){
  if(p->isize == 0){
    if(size > p->size) return NULL;
    p->front[0] = 0;
    p->back[0] = size;
    p->isize ++;
    return p->data;
  }
  int this = -1;
  if(p->front[0] >= size){
    this = 0;
  }
  for(int i = 1; i < p->isize; i++){
    int len = p->front[i] - p->back[i-1];
    if(len >= size){
      this = i;
      break;
    }
  }
  if(this!=-1){
    p->isize++;
    for(int i = p->isize - 1; i >= this; i--){
      p->front[i + 1] = p->front[i];
      p->back[i + 1] = p->back[i];
    }
    if(this == 0){
      p->front[0] = 0;
      p->back[0] = size;
      return p->data;
    }
    p->front[this] = p->back[this-1];
    p->back[this] = p->front[this]+size;
    return p->data+p->front[this];
  }
  if(size > p->size - p->back[p->isize - 1]) return NULL;
  p->front[p->isize] = p->back[p->isize - 1];
  p->back[p->isize] = p->front[p->isize] + size;
  p->isize++;
  return p->data+p->front[p->isize - 1];
}

bool pool_free(struct pool *p, char *addr){
  int flag = addr - p->data;
  int here = -1;
  bool sub = false;
  for(int i = 0; i < p->isize; i++){
    if(p->back[i] > flag&&p->front[i]<flag){
      here = i;
      break;
    }
    if(p->front[i]==flag){
      sub = true;
      here = i;
    }
  }
  if(here == -1) return false;
  p->back[here] = flag;
  if(sub){
    for(int i = here+1; i < p->isize; i++){
      p->front[i-1] = p->front[i];
      p->back[i-1] = p->back[i];
    }
    p->isize--;
  }
  return true;
}

char *pool_realloc(struct pool *p, char *addr, int size){
  int flag = addr - p->data;
  int here = -1;
  bool start = false;
  for(int i = 0; i < p->isize; i++){
    if(p->back[i] > flag&&p->front[i]<=flag){
      if(p->front[i]==flag)
        start = true;
      here = i;
      break;
    }
  }
  if(here == -1) return NULL;
  int old = p->back[here] - flag;
  if(old > size){
    pool_free(p, addr+size);
    return addr;
  }
  if(p->front[here+1] - flag>=size){
    p->back[here] += size - old;
    return addr;
  }
  else{
    if(start){
      if(here == 0){
        if(p->front[1] >= size){
          int pointer = 0;
          for(int i = p->front[0]; i < p->back[0]; i++){
            p->data[pointer] = p->data[i];
            pointer++;
          }
          for(int i = p->back[0]; i < size; i++){
            p->data[i] = 0;
          }
          pool_free(p, addr);
          return pool_alloc(p, size);
        }
        int this = -1;
        for(int i = 1; i < p->isize; i++){
          if(p->front[i]-p->back[i-1] >= size){
            this = i;
            break;
          }
        }
        if(this == -1){
          if(p->size - p->back[p->isize - 1] >= size){
            int pointer = p->back[p->isize - 1];
            for(int i = p->front[here]; i < p->back[here]; i++){
              p->data[pointer] = p->data[i];
              pointer++;
            }
            pool_free(p, addr);
            return pool_alloc(p, size);
          }
          else return NULL;
        }
        else{
          int pointer = p->back[this - 1];
          for(int i = p->front[here]; i < p->back[here]; i++){
            p->data[pointer] = p->data[i];
            pointer++;
          }
          pool_free(p, addr);
          return pool_alloc(p, size);
        }
      }
      if(p->front[0] >= size){
        int pointer = 0;
        for(int i = p->front[here]; i < p->back[here]; i++){
          p->data[pointer] = p->data[i];
          pointer++;
        }
        pool_free(p, addr);
        return pool_alloc(p, size);
      }
      int this = -1;
      int front = p->front[here];
      int back = p->back[here];
      p->front[here] = p->back[here-1];
      p->back[here] = p->back[here-1];
      for(int i = 1; i < p->isize; i++){
        if(p->front[i]-p->back[i-1] >= size){
          this = i;
          break;
        }
      }
      p->front[here] = front;
      p->back[here] = back;
      if(this == -1){
        if(p->size - p->back[p->isize - 1] >= size){
          int pointer = p->back[p->isize - 1];
          for(int i = p->front[here]; i < p->back[here]; i++){
            p->data[pointer] = p->data[i];
            pointer++;
          }
          pool_free(p, addr);
          return pool_alloc(p, size);
        }
        else return NULL;
      }
      else{
        if(this != here + 1){
          int pointer = p->back[this - 1];
          for(int i = p->front[here]; i < p->back[here]; i++){
            p->data[pointer] = p->data[i];
            pointer++;
          }
        }
        else{
          int pointer = p->back[this - 2];
          for(int i = p->front[here]; i < p->back[here]; i++){
            p->data[pointer] = p->data[i];
            pointer++;
          }
          for(int i = p->back[here]; i < p->front[here] + size; i++){
            p->data[i] = 0;
          }
        }
        pool_free(p, addr);
        return pool_alloc(p, size);
      }
    }
    else{
      return NULL;
    }
  }
}

void pool_print_active(struct pool *p){
  if(p->isize==0){
    printf("active: none\n");
    return;
  }
  printf("active: %d [%d]", p->front[0], p->back[0]-p->front[0]);
  for(int i = 1; i < p->isize; i++){
    printf(", %d [%d]", p->front[i], p->back[i]-p->front[i]);
  }
  printf("\n");
}

void pool_print_available(struct pool *p){
  if(p->isize == 0){
    printf("available: %d [%d]\n", 0, p->size);
    return;
  }
  int start = -1;
  if(p->front[0]!=0){
    start = 0;
  }
  else{
    for(int i = 1; i < p->isize; i++){
      if(p->back[i-1]!=p->front[i]){
        start = i;
        break;
      }
    }
  }
  if(start == -1){
    if(p->back[p->isize - 1]!=p->size){
      printf("available: %d [%d]\n", p->back[p->isize - 1], p->size - p->back[p->isize - 1]);
      return;
    }
    else{
      printf("available: none\n");
      return;
    }
  }
  if(start == 0){
    printf("available: %d [%d]", 0, p->front[0]);
  }
  else{
    printf("available: %d [%d]", p->back[start-1], p->front[start]-p->back[start-1]);
  }
  for(int i = start + 1; i < p->isize; i++){
    if(p->back[i-1]!=p->front[i]){
      printf(", %d [%d]", p->back[i-1], p->front[i] - p->back[i-1]);
    }
  }
  if(p->back[p->isize - 1]!=p->size){
    printf(", %d [%d]\n", p->back[p->isize - 1], p->size - p->back[p->isize - 1]);
  }
  else{
    printf("\n");
  }
}
